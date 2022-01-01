#include <bit>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

const int PWLEN_MAX = 16;
const int NUM_CHARSET = 42;

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

unsigned char itoa[NUM_CHARSET] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
    0x0d, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x18, 0x19, 0x1a, 0x1b,
    0x1c, 0x1d, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2a,
    0x2b, 0x2c, 0x2d, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

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

struct alignas(4) AdditionalInfo {
  unsigned short partial_f7;
  unsigned char partial_fb;
  unsigned char partial_f9;
};

struct Node {
  Digits digits;
  int depth;
  std::string pw;
  AdditionalInfo info;
};

struct BackwardInfo {
  BackwardInfo *next;
  char pw[PWLEN_MAX / 2];
  AdditionalInfo info;
};

size_t get_index(unsigned int f4, unsigned int f5, unsigned int f8,
                 unsigned int fa) {
  size_t ret = (f4 << 24) | (f5 << 16) | (f8 << 8) | fa;
  return ret;
}

BackwardInfo **read_from_file(const char *filename, int backward_len) {
  // yokai_bwで作ったファイルを読み込む
  printf("sizeof fpos_t:%lu string:%lu\n", sizeof(fpos_t), sizeof(std::string));
  printf("sizeof BackwardInfo:%lu AdditionalInfo:%lu\n", sizeof(BackwardInfo),
         sizeof(AdditionalInfo));
  printf("Read from file start\n");
  BackwardInfo **list = (BackwardInfo **)calloc(
      sizeof(BackwardInfo *), static_cast<size_t>(256) * 256 * 256 * 256);
  printf(" calloc done.\n");

  FILE *fp = fopen(filename, "rb");
  unsigned long long counter = 0;
  while (!feof(fp)) {
    counter++;
    if (counter % 1000000 == 0) {
      printf(" counter: %llu\n", counter);
    }
    int len = 4 + 4 + backward_len;
    unsigned char buffer[len];
    BackwardInfo *p_bw_info = new BackwardInfo();
    BackwardInfo &bw_info = *p_bw_info;
    bw_info.next = nullptr;

    fread(buffer, len, 1, fp);
    unsigned char f4 = buffer[0];
    unsigned char f5 = buffer[1];
    unsigned char f8 = buffer[2];
    unsigned char fa = buffer[3];
    bw_info.info.partial_f7 = (buffer[4] << 8) | buffer[5];
    bw_info.info.partial_f9 = buffer[6];
    bw_info.info.partial_fb = buffer[7];
    memcpy(bw_info.pw, &buffer[8], backward_len);
    bw_info.pw[backward_len] = '\0';
    size_t index = get_index(f4, f5, f8, fa);
    if (list[index]) {
      p_bw_info->next = list[index];
    }
    list[index] = p_bw_info;
  }

  fclose(fp);
  printf("Read from file done. count: %llu\n", counter);
  return list;
}

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
  int backward_len = atk_count / 2;
  printf("Backward_len: %d\n", backward_len);

  std::vector<Node> pool;

  // 最初を入れる
  Node start_node;
  start_node.depth = 0;
  start_node.pw = std::string("");
  start_node.info.partial_f7 = 0;
  start_node.info.partial_f9 = 0;
  start_node.info.partial_fb = 0;
  start_node.digits.f4 = 0;
  start_node.digits.f5 = 0;
  start_node.digits.f6 = atk_count;
  start_node.digits.f7 = 0;
  start_node.digits.f8 = 0;
  start_node.digits.f9 = 0;
  start_node.digits.fa = 0;
  start_node.digits.fb = 0;

  char filename[128];
  snprintf(filename, 8 * 2 + 5, "%02X%02X%02X%02X%02X%02X%02X%02X.dat", atk31F4,
           atk31F5, atk_count, atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);

  auto backward_list = read_from_file(filename, atk_count / 2);

  pool.push_back(start_node);

  // 探索
  unsigned long long count = 0;
  unsigned long long found_count = 0;
  while (!pool.empty()) {
    count++;
    Node node = pool.back();
    pool.pop_back();

    // 目標深さに到達
    if (node.depth >= atk_count - backward_len) {
      size_t index = get_index(node.digits.f4, node.digits.f5, node.digits.f8,
                               node.digits.fa);
      BackwardInfo *p = backward_list[index];
      // if (p != NULL) {
      //   printf("found digits in backward list. pw:%s\n", node.pw.c_str());
      // }
      while (p != NULL) {
        // printf("target depth reached: count: %llu depth:%d pw: %s index:%lu "
        //        "list[index]:%p\n",
        //        count, node.depth, node.pw.c_str(), index, p);
        AdditionalInfo &info = p->info;
        if ((((node.digits.f7 + info.partial_f7) & 0xff) == atk31F7) &&
            ((node.digits.f9 ^ info.partial_f9) == atk31F9) &&
            (node.digits.fb + info.partial_fb == atk31FB)) {
          // 見つかった
          printf("Hit: %s\n", (node.pw + p->pw).c_str());
        }
        p = p->next;
      }
      // これ以上深く探索しない
      continue;
    }

    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char c = itoa[i];
      Node new_node = forward_step(node, c);
      // printf("Create new node. depth:%d c:%02X old_pw:%s new_pw:%s\n",
      //        new_node.depth, c, node.pw.c_str(), new_node.pw.c_str());
      pool.push_back(std::move(new_node));
    }
  }
  printf("End, count: %llu found_count: %llu\n", count, found_count);
  return 0;
}
