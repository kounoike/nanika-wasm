#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

const int PWLEN_MAX = 16;
const int NUM_CHARSET_nmc = 42;
const int NUM_CHARSET = 39;
const int BULK_SIZE = 64;
const int BWLEN_MAX = 6;

// 文字コード変換テーブル
char atoy[256] = {
    'A', 'H', 'O', 'V', '1', '6', '*', '*', 'B', 'I', 'P', 'W', '2', '7', '*',
    '*', 'C', 'J', 'Q', 'X', '3', '8', '*', '*', 'D', 'K', 'R', 'Y', '4', '9',
    '*', '*', 'E', 'L', 'S', 'Z', '5', '0', '*', '*', 'F', 'M', 'T', '-', 'n',
    '!', '*', '*', 'G', 'N', 'U', '.', 'm', 'c', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    '*'};

// nmc入り
unsigned char itoa_nmc[NUM_CHARSET_nmc] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2a,
    0x2b, 0x2c, 0x2d, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

// nmcなし
unsigned char itoa[NUM_CHARSET] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x18, 0x19,
    0x1a, 0x1b, 0x1c, 0x1d, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x28, 0x29, 0x2a, 0x2b, 0x2d, 0x30, 0x31, 0x32, 0x33};

// 文字のビットを上下反転するテーブル
unsigned char lut_reverse_char_bit[] = {
    0b00000000, 0b10000000, 0b01000000, 0b11000000, 0b00100000, 0b10100000,
    0b01100000, 0b11100000, 0b00010000, 0b10010000, 0b01010000, 0b11010000,
    0b00110000, 0b10110000, 0b01110000, 0b11110000, 0b00001000, 0b10001000,
    0b01001000, 0b11001000, 0b00101000, 0b10101000, 0b01101000, 0b11101000,
    0b00011000, 0b10011000, 0b01011000, 0b11011000, 0b00111000, 0b10111000,
    0b01111000, 0b11111000, 0b00000100, 0b10000100, 0b01000100, 0b11000100,
    0b00100100, 0b10100100, 0b01100100, 0b11100100, 0b00010100, 0b10010100,
    0b01010100, 0b11010100, 0b00110100, 0b10110100, 0b01110100, 0b11110100,
    0b00001100, 0b10001100, 0b01001100, 0b11001100, 0b00101100, 0b10101100,
    0b01101100, 0b11101100, 0b00011100, 0b10011100, 0b01011100, 0b11011100,
    0b00111100, 0b10111100, 0b01111100,
    0b11111100, // 0x35まであればいいからこのくらい。
};

struct Digits {
  unsigned char f4;
  unsigned char f5;
  unsigned char f6;
  unsigned char f7;
  unsigned char f8;
  unsigned char f9;
  unsigned char fa;
  unsigned char fb;
};

struct Node {
  Digits digits;
  int depth;
  std::string pw;
};

struct BackwardInfo {
  unsigned char f4;
  unsigned char f5;
  unsigned char f8;
  unsigned char fa;
  unsigned char partial_f7;
  unsigned char partial_f9;
  unsigned char partial_fb;
  char pw[BWLEN_MAX];
};

struct ThreadInfo {
  int thread_id;
  bool is_finished;
  Node start_node;
};

bool bwi_comp(const BackwardInfo &a, const BackwardInfo &b) {
  if (a.f4 != b.f4)
    return a.f4 < b.f4;
  else if (a.f5 != b.f5)
    return a.f5 < b.f5;
  else if (a.f8 != b.f8)
    return a.f8 < b.f8;
  else
    return a.fa < b.fa;
};

Node forward_step(const Node &node, unsigned char p) {
  // 基本情報
  Node new_node(node);
  new_node.pw = node.pw + atoy[p];
  new_node.depth = node.depth + 1;
  const Digits &prev = node.digits;
  Digits &next = new_node.digits;

  unsigned int A, C, C1;
  // $31F4, $31F5
  next.f4 = prev.f4;
  next.f5 = prev.f5;
  A = p;

  for (int Y = 0; Y < 8; Y++) {
    C = A & 0x80;
    A <<= 1;

    C1 = next.f4 & 0x01;
    next.f4 = (next.f4 >> 1) | C;
    C = next.f5 & 0x01;
    next.f5 = (next.f5 >> 1) | (C1 << 7);

    next.f4 ^= C > 0 ? 0x84 : 0;
    next.f5 ^= C > 0 ? 0x08 : 0;
  }

  // $31F7, $31F8
  C = next.f4 >= 0xE5 ? 1 : 0;
  A = p + prev.f7 + C;
  if (A > 0xff) {
    A &= 0xff;
    C = 1;
  } else {
    C = 0;
  }
  next.f7 = A;

  A = prev.f8 + next.f5 + C;
  if (A > 0xff) {
    A &= 0xff;
    C = 1;
  } else {
    C = 0;
  }
  next.f8 = A;

  // $31F9: xor
  next.f9 = prev.f9 ^ p;

  // $31FA
  C1 = prev.fa & 0x01;
  A = (prev.fa >> 1) | (C << 7); // $31F8計算時のCがここで入る
  A = p + A + C1;
  if (A > 0xff) {
    A &= 0xff;
    C = 1;
  } else {
    C = 0;
  }
  next.fa = A;
  // printf("forward pw:[%s] c:%02X[%c] prev.fa:[%02X] c_f8:[%d]
  // next.fa:[%02X]\n",
  //        node.pw.c_str(), p, atoy[p], prev.fa, c_f8, next.fa);

  // $31FB: popcount + C
  next.fb = prev.fb + std::popcount(p) + C;

  return new_node;
}

std::vector<Node> forward_step_simd(const Node &node) {
  const Digits &prev = node.digits;
  unsigned char p[BULK_SIZE];
  unsigned char A[BULK_SIZE], A1[BULK_SIZE], C[BULK_SIZE], C1[BULK_SIZE];

  for (int idx = 0; idx < BULK_SIZE; idx++)
    p[idx] = idx;

  unsigned char f4[BULK_SIZE], f5[BULK_SIZE], f7[BULK_SIZE], f8[BULK_SIZE],
      f9[BULK_SIZE], fa[BULK_SIZE], fb[BULK_SIZE];

  // $31F4, $31F5
  for (int idx = 0; idx < BULK_SIZE; idx++)
    f4[idx] = prev.f4;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    f5[idx] = prev.f5;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = p[idx];

  for (int Y = 0; Y < 8; Y++) {
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = A[idx] & 0x80;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A[idx] <<= 1;

    for (int idx = 0; idx < BULK_SIZE; idx++)
      C1[idx] = f4[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      f4[idx] = (f4[idx] >> 1) | C[idx];
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = f5[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      f5[idx] = (f5[idx] >> 1) | (C1[idx] << 7);

    for (int idx = 0; idx < BULK_SIZE; idx++)
      f4[idx] ^= C[idx] > 0 ? 0x84 : 0;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      f5[idx] ^= C[idx] > 0 ? 0x08 : 0;
  }

  // $31F7, $31F8
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = f4[idx] >= 0xE5 ? 1 : 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = p[idx] + prev.f7 + C[idx];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = A[idx] < prev.f7 ? 1 : 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    f7[idx] = A[idx];

  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = prev.f8 + f5[idx] + C[idx];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = (A[idx] < prev.f8 || (C[idx] > 0 && prev.f8 == A[idx]));
  for (int idx = 0; idx < BULK_SIZE; idx++)
    f8[idx] = A[idx];

  // $31F9: xor
  for (int idx = 0; idx < BULK_SIZE; idx++)
    f9[idx] = prev.f9 ^ p[idx];

  // $31FA
  unsigned char C2 = prev.fa & 0x01;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = (prev.fa >> 1) | (C[idx] << 7); // $31F8計算時のCがここで入る
  for (int idx = 0; idx < BULK_SIZE; idx++)
    fa[idx] = p[idx] + A[idx] + C2;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = fa[idx] < A[idx];

  // printf("forward pw:[%s] c:%02X[%c] prev.fa:[%02X] c_f8:[%d]
  // next.fa:[%02X]\n",
  //        node.pw.c_str(), p, atoy[p], prev.fa, c_f8, next.fa);

  // $31FB: popcount + C
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A1[idx] = p[idx];
  for (int Y = 0; Y < 6; Y++) {
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A[idx] += A1[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A1[idx] >>= 1;
  }
  for (int idx = 0; idx < BULK_SIZE; idx++)
    fb[idx] = prev.fb + A[idx] + C[idx];

  std::vector<Node> ret;
  for (int i = 0; i < NUM_CHARSET; i++) {
    unsigned char c = itoa[i];
    Node new_node(node);
    new_node.pw = node.pw + atoy[c];
    new_node.depth = node.depth + 1;
    new_node.digits.f4 = f4[c];
    new_node.digits.f5 = f5[c];
    new_node.digits.f6 = prev.f6;
    new_node.digits.f7 = f7[c];
    new_node.digits.f8 = f8[c];
    new_node.digits.f9 = f9[c];
    new_node.digits.fa = fa[c];
    new_node.digits.fb = fb[c];
    ret.push_back(std::move(new_node));
  }

  return ret;
}

void calc_thread(ThreadInfo &ti, std::mutex &mtx,
                 std::vector<BackwardInfo> &backward_vector, FILE *result_fp,
                 int atk_count, int fbmin, int fbmax, int backward_len,
                 int atk31F7, int atk31F9, int atk31FB, char *basepart) {
  std::vector<Node> pool;
  pool.push_back(ti.start_node);

  // 探索
  unsigned long long count = 0;
  unsigned long long found_count = 0;
  while (!pool.empty()) {
    count++;
    Node node = pool.back();
    pool.pop_back();

    // $31FBベース枝狩り
    int rem_chars = atk_count - node.depth;
    const unsigned char &fb = node.digits.fb;
    // if (fb > atk31FB || fb + 5 * rem_chars < atk31FB) {
    //   continue;
    // }
    if (fb + fbmin > atk31FB ||
        fb + 5 * (rem_chars - backward_len) + fbmax < atk31FB) {
      continue;
    }

    // 目標深さに到達
    if (node.depth >= atk_count - backward_len) {
      BackwardInfo bwi_for_comp;
      bwi_for_comp.f4 = node.digits.f4;
      bwi_for_comp.f5 = node.digits.f5;
      bwi_for_comp.f8 = node.digits.f8;
      bwi_for_comp.fa = node.digits.fa;

      auto p = std::equal_range(backward_vector.begin(), backward_vector.end(),
                                bwi_for_comp, bwi_comp);
      for (auto it = p.first; it != p.second; it++) {
        if ((((node.digits.f7 + it->partial_f7) & 0xff) == atk31F7) &&
            ((node.digits.f9 ^ it->partial_f9) == atk31F9) &&
            (node.digits.fb + it->partial_fb == atk31FB)) {
          // 見つかった
          std::lock_guard<std::mutex> lock(mtx);
          char pw[PWLEN_MAX];
          memset(pw, 0x00, PWLEN_MAX);
          memcpy(pw, it->pw, backward_len);
          // printf("Hit: %s\n", (node.pw + pw).c_str());
          fputs((node.pw + pw + "\n").c_str(), result_fp);
          found_count++;
        }
      }
      // これ以上深く探索しない
      continue;
    }

    std::vector<Node> new_nodes = forward_step_simd(node);
    pool.insert(pool.end(), new_nodes.begin(), new_nodes.end());
  }
  ti.is_finished = true;
}

int main(int argc, char *argv[]) {
  int atk_count = 1;
  int atk31F4 = 0, atk31F5 = 0, atk31F7 = 0, atk31F8 = 0, atk31F9 = 0,
      atk31FA = 0, atk31FB = 0;

  // アタック目標値を設定
  if (argc < 9) {
    printf("yokai_bw - Backward search table creator\n");
    printf("usage:yokai_bw $31F4 $31F5 $31F6 $31F7 $31F8 $31F9 $31FA $31FB\n");
    return 0;
  }

  // 引数を各ターゲットに割り当て
  atk31F4 = (int)strtoul(argv[1], NULL, 16);
  atk31F5 = (int)strtoul(argv[2], NULL, 16);
  atk_count = (int)strtol(argv[3], NULL, 16);
  atk31F7 = (int)strtoul(argv[4], NULL, 16);
  atk31F8 = (int)strtoul(argv[5], NULL, 16);
  atk31F9 = (int)strtoul(argv[6], NULL, 16);
  atk31FA = (int)strtoul(argv[7], NULL, 16);
  atk31FB = (int)strtoul(argv[8], NULL, 16);

  int prefix_count = 0;
  char prefix[PWLEN_MAX];
  if (argc > 9) {
    prefix_count = argc - 9;
    printf("プレフィクス指定で動作します prefix: ");
    for (int i = 0; i < prefix_count; i++) {
      prefix[i] = (unsigned char)strtoul(argv[9 + i], NULL, 16);
      printf("%02X ", prefix[i]);
    }
    printf("\n");
  } else {
    printf("最初からスタートします\n");
  }

  int backward_len = std::min(atk_count / 2, BWLEN_MAX);
  printf("Backward_len: %d\n", backward_len);

  // 最初を入れる
  Node start_node;
  start_node.depth = 0;
  start_node.pw = std::string("");
  start_node.digits.f4 = 0;
  start_node.digits.f5 = 0;
  start_node.digits.f6 = atk_count;
  start_node.digits.f7 = 0;
  start_node.digits.f8 = 0;
  start_node.digits.f9 = 0;
  start_node.digits.fa = 1;
  start_node.digits.fb = 0;

  // プレフィックス対応
  for (int i = 0; i < prefix_count; i++) {
    unsigned char p = prefix[i];
    start_node = forward_step(start_node, p);
  }

  char basepart[256];
  snprintf(basepart, 256, "dat_%02X%02X%02X%02X%02X%02X%02X%02X", atk31F4,
           atk31F5, atk_count, atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);

  char minmax_filename[256];
  snprintf(minmax_filename, 256, "%s/minmax.dat", basepart);
  FILE *minmax_fp = fopen(minmax_filename, "rb");
  unsigned char minmax_buffer[2];
  fread(minmax_buffer, 2, 1, minmax_fp);
  fclose(minmax_fp);
  int fbmin = minmax_buffer[0];
  int fbmax = minmax_buffer[1];

  char backward_filename[256];
  snprintf(backward_filename, 256, "%s/backward.dat", basepart);
  FILE *backward_fp = fopen(backward_filename, "rb");

  auto num =
      std::filesystem::file_size(backward_filename) / sizeof(BackwardInfo);
  if (!backward_fp) {
    printf("backward.dat オープンエラー\n");
    return 0;
  }

  // ソートされたbackward探索結果
  printf("backward.dat 読み込み %lu件\n", num);
  std::vector<BackwardInfo> backward_vector(num);
  auto read_num =
      fread(&backward_vector[0], sizeof(BackwardInfo), num, backward_fp);
  assert(read_num == num);
  fclose(backward_fp);

  // 結果ファイルオープン
  char result_filename[256];
  snprintf(result_filename, 256, "%s/result%s%s.txt", basepart,
           prefix_count > 0 ? "_" : "", start_node.pw.c_str());
  FILE *result_fp = fopen(result_filename, "w");
  if (!result_fp) {
    printf("結果ファイルオープンエラー\n");
    return 0;
  }

  std::vector<ThreadInfo> thread_info_vector;
  for (int i = 0; i < NUM_CHARSET; i++) {
    unsigned char p1 = itoa[i];
    Node node1 = forward_step(start_node, p1);
    for (int j = 0; j < NUM_CHARSET; j++) {
      unsigned char p2 = itoa[j];
      Node node2 = forward_step(node1, p2);
      ThreadInfo ti;
      ti.is_finished = false;
      ti.start_node = node2;
      ti.thread_id = i * NUM_CHARSET + j;
      thread_info_vector.push_back(std::move(ti));
    }
  }

  int create_count = 0;
  int cpu_count = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  std::vector<int> current_targets;
  std::mutex mtx;

  // 最初はCPUコア数分作る
  for (int i = 0; i < cpu_count; i++) {
    threads.push_back(std::thread([&, create_count]() {
      calc_thread(thread_info_vector[create_count], mtx, backward_vector,
                  result_fp, atk_count, fbmin, fbmax, backward_len, atk31F7,
                  atk31F9, atk31FB, basepart);
    }));
    current_targets.push_back(create_count);
    create_count++;
  }

  // 5us間隔でポーリングして終わり次第、次を始めさせる
  while (create_count < NUM_CHARSET * NUM_CHARSET) {
    for (int i = 0; i < cpu_count; i++) {
      if (thread_info_vector[current_targets[i]].is_finished) {
        threads[i].join();
        threads[i] = std::thread([&, create_count]() {
          calc_thread(thread_info_vector[create_count], mtx, backward_vector,
                      result_fp, atk_count, fbmin, fbmax, backward_len, atk31F7,
                      atk31F9, atk31FB, basepart);
        });
        current_targets[i] = create_count;
        create_count++;
        if (create_count >= NUM_CHARSET * NUM_CHARSET) {
          break;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(5));
  }
  {
    std::lock_guard<std::mutex> lock(mtx);
    printf("あとは全部待つ\n");
  }

  // 最後は全部終わるまで待つ
  for (int i = 0; i < cpu_count; i++) {
    threads[i].join();
  }

  printf("End\n");
  fclose(result_fp);
  return 0;
}
