#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

const int PWLEN_MAX = 16;
const int NUM_CHARSET_nmc = 42;
const int NUM_CHARSET = 39;
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

struct AdditionalInfo {
  unsigned char partial_f7;
  unsigned char partial_fb;
  unsigned char partial_f9;
};

struct Node {
  Digits digits;
  AdditionalInfo info;
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

// パスワードと次ステップの$31F4を入れると前のステップの$31F5が出るテーブル
unsigned char reverse_lut_f5[64][256];
// 前ステップの$31F5と前ステップの$31F5を入れると前ステップの$31F4が出るテーブル
unsigned char reverse_lut_f4[256][256];

// パスワード・次ステップの$31F4・次ステップの$31F5を入れると前のステップの
// $31F4, $31F5が出るテーブル
// X & 0x00010000 >> 16:
//   正しい値が入っている（初期化済み）: 1
//   未初期化: 0
// X & 0x0000ff00 >> 8: 前のステップの$31F4
// X & 0x000000ff:前のステップの$31F5
unsigned int reverse_lut_f4f5[64][256][256];

// bit0の0/1、$31F8計算時のキャリーフラグ、パスワード・次ステップの$31FAを入れると
// 前のステップの$31FAが出るテーブル
// X = reverse_lut_fa[b0][C_f8][pw][next_fa]
// X & 0x200 >> 9: 初期化済み: 1 未初期化: 0
// X & 0x100 >> 8: $31FB計算に流れる$31FA計算時のキャリーフラグ
// X & 0xff: 前のステップの$31FA
unsigned int reverse_lut_fa[2][2][64][256];

void create_lut() {
  for (int c_f8 = 0; c_f8 < 2; c_f8++) {
    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char p = itoa[i];
      for (int j = 0; j < 256; j++) {
        unsigned char prev_fa = j;
        int A, C, C1;
        // $31FA
        C1 = prev_fa & 0x01;
        A = prev_fa >> 1 | (c_f8 << 7);
        A = p + A + C1;
        if (A > 0xff) {
          C = 1;
        } else {
          C = 0;
        }
        int next_fa = A & 0xff;
        assert((reverse_lut_fa[C1][c_f8][p][next_fa] & 0x200) == 0);
        reverse_lut_fa[C1][c_f8][p][next_fa] = 0x200 | (C << 8) | prev_fa;
      }
    }
  }
  for (int i = 0; i < NUM_CHARSET; i++) {
    for (int j = 0; j < 256; j++) {
      for (int k = 0; k < 256; k++) {
        unsigned char p = itoa[i];
        unsigned int A, C, C1;
        unsigned char prev_f4 = j;
        unsigned char prev_f5 = k;

        A = p;
        unsigned char next_f4 = prev_f4;
        unsigned char next_f5 = prev_f5;

        for (int Y = 0; Y < 8; Y++) {
          C = A & 0x80;
          A <<= 1;

          C1 = next_f4 & 0x01;
          next_f4 = (next_f4 >> 1) | C;
          C = next_f5 & 0x01;
          next_f5 = (next_f5 >> 1) | (C1 << 7);

          next_f4 ^= C > 0 ? 0x84 : 0;
          next_f5 ^= C > 0 ? 0x08 : 0;
        }
        reverse_lut_f5[p][next_f4] = prev_f5;
        reverse_lut_f4[prev_f5][next_f5] = prev_f4;
        reverse_lut_f4f5[p][next_f4][next_f5] =
            0x10000 | (prev_f4 << 8) | prev_f5;
      }
    }
  }
}

// 検算用
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

  // $31FB: popcount + C
  next.fb = prev.fb + std::popcount(p) + C;

  return new_node;
}

std::vector<Node> backward_step(const Node &node, unsigned char p) {
  const Digits &next = node.digits;
  Digits prev;
  const AdditionalInfo &info = node.info;

  // 返却用
  std::vector<Node> ret;

  // $31F6は変化なし
  prev.f6 = next.f6;

  // $31F4, $31F5
  unsigned int f4f5 = reverse_lut_f4f5[p][next.f4][next.f5];
  assert((f4f5 & 0x10000) == 0x10000);
  prev.f4 = (f4f5 >> 8) & 0xff;
  prev.f5 = f4f5 & 0xff;

  // 前ステップの$31F7は次ステップの$31F7, 次ステップの$31F4と0xE5の比較で逆算
  int f7 = next.f7 - p - (next.f4 >= 0xE5 ? 1 : 0);
  int c_f7 = 0; // f7計算した後のキャリー
  if (f7 < 0) {
    f7 += 256;
    c_f7 = 1;
  }
  prev.f7 = f7 & 0xff;

  // 前$31F8の逆算
  int f8 = next.f8 - next.f5 - c_f7;
  int c_f8 = 0;
  if (f8 < 0) {
    f8 += 256;
    c_f8 = 1;
  }
  prev.f8 = f8 & 0xff;

  // 前$31f9はxorなのでそのままで逆算できる
  prev.f9 = next.f9 ^ p;

  // 前$31FAの逆算はちょっとややこしい
  // ROR → PWとADCの2命令で求めている
  // つまり、前$31FAのbit0と前$31FAの右1bitローテートを加算している。
  // そのため、0/1で分岐する必要がある
  // その代わり、次$31FA - PWで出てくる値と、
  // $31F8計算時のCの値が一致しなければ枝狩りが可能

  // bit0の0/1で分岐
  for (int bit0 = 0; bit0 < 2; bit0++) {
    unsigned int val = reverse_lut_fa[bit0][c_f8][p][next.fa];
    if ((val & 0x200) == 0) {
      // 未初期化（c_f8と矛盾しているなど）の場合スキップ
      continue;
    }
    int c_fa = (val & 0x100) >> 8;
    prev.fa = val & 0xff;

    // 前$31FBの逆算はpopcount+$31FA計算のキャリー
    if (next.fb < std::popcount(p) + c_fa) {
      // 文字数*(4(popcount最大) + 1(キャリー))から
      // $31FBの計算ではオーバーフローはしない。
      continue;
    }
    prev.fb = next.fb - std::popcount(p) - c_fa;

    Node new_node;
    new_node.depth = node.depth + 1;
    new_node.pw = atoy[p] + node.pw;
    // 計算した前チェックデジット
    new_node.digits.f4 = prev.f4;
    new_node.digits.f5 = prev.f5;
    new_node.digits.f6 = prev.f6;
    new_node.digits.f7 = prev.f7;
    new_node.digits.f8 = prev.f8;
    new_node.digits.f9 = prev.f9;
    new_node.digits.fa = prev.fa;
    new_node.digits.fb = prev.fb;

    // 付加情報の生成
    AdditionalInfo &ret_info = new_node.info;
    ret_info.partial_f7 =
        (info.partial_f7 + p + (next.f4 >= 0xE5 ? 1 : 0)) & 0xff;
    ret_info.partial_f9 = info.partial_f9 ^ p;
    ret_info.partial_fb = info.partial_fb + std::popcount(p) + c_fa;

    ret.push_back(std::move(new_node));
  }

  return ret;
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

  printf("LUTの計算開始\n");
  create_lut();
  printf("LUTの計算完了\n");

  // 最初を入れる
  Node start_node;
  start_node.depth = 0;
  start_node.pw = std::string("");
  start_node.info.partial_f7 = 0;
  start_node.info.partial_f9 = 0;
  start_node.info.partial_fb = 0;
  start_node.digits.f4 = atk31F4;
  start_node.digits.f5 = atk31F5;
  start_node.digits.f6 = atk_count;
  start_node.digits.f7 = atk31F7;
  start_node.digits.f8 = atk31F8;
  start_node.digits.f9 = atk31F9;
  start_node.digits.fa = atk31FA;
  start_node.digits.fb = atk31FB;

  char basename[256];
  snprintf(basename, 256, "dat_%02X%02X%02X%02X%02X%02X%02X%02X", atk31F4,
           atk31F5, atk_count, atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);
  if (std::filesystem::exists(basename)) {
    printf("%s: 既に作成済みです。再生成するときは一度消してください。\n",
           basename);
    return 0;
  }
  std::filesystem::create_directories(basename);

  std::vector<Node> pool;

  pool.push_back(std::move(start_node));

  // 探索
  unsigned long long count = 0;
  unsigned long long found_count = 0;
  int backward_len = std::min(atk_count / 2, BWLEN_MAX);

  int fbmax = 0;
  int fbmin = 10 * atk_count; // 適当に大きく。INT_MAX出してくるほどでもない

  std::vector<BackwardInfo> bwi_vector;

  while (!pool.empty()) {
    count++;
    Node node = pool.back();
    pool.pop_back();

    // $31FBが頑張っても足りないときは枝狩り
    // オーバーしちゃってる場合のは別の場所
    if (node.digits.fb > 5 * (atk_count - node.depth)) {
      continue;
    }

    // 目標深さに到達
    if (node.depth >= backward_len) {
      found_count++;

      std::pair<unsigned char, unsigned char> key{node.digits.f4,
                                                  node.digits.f5};

      BackwardInfo bwi;
      bwi.f4 = node.digits.f4;
      bwi.f5 = node.digits.f5;
      bwi.f8 = node.digits.f8;
      bwi.fa = node.digits.fa;
      bwi.partial_f7 = node.info.partial_f7;
      bwi.partial_f9 = node.info.partial_f9;
      bwi.partial_fb = node.info.partial_fb;
      memcpy(bwi.pw, node.pw.c_str(), backward_len);

      bwi_vector.push_back(std::move(bwi));

      unsigned char fb = node.info.partial_fb & 0xff;
      if (fbmax < fb) {
        fbmax = fb;
      }
      if (fbmin > fb) {
        fbmin = fb;
      }
      continue;
    }

    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char c = itoa[i];
      std::vector<Node> new_nodes = backward_step(node, c);
      // for (auto n : new_nodes) {
      //   Node n2 = forward_step(n, c);
      //   if (node.digits.f4 != n2.digits.f4) {
      //     printf("F4 mismatch!: pw:%s c:%02X[%c] node.f4:[%02X] "
      //            "bw.f4:[%02X] bw->fw.f4:[%02X] bw.f5:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.f4, n.digits.f4,
      //            n2.digits.f4, n.digits.f5);
      //   }
      //   if (node.digits.f5 != n2.digits.f5) {
      //     printf("F5 mismatch!: pw:%s c:%02X[%c] node.f5:[%02X] "
      //            "bw.f5:[%02X] bw->fw.f5:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.f5, n.digits.f5,
      //            n2.digits.f5);
      //   }
      //   if (node.digits.f7 != n2.digits.f7) {
      //     printf("F7 mismatch!: pw:%s c:%02X[%c] node.f7:[%02X] "
      //            "bw.f7:[%02X] bw->fw.f7:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.f7, n.digits.f7,
      //            n2.digits.f7);
      //   }
      //   if (node.digits.f8 != n2.digits.f8) {
      //     printf("F8 mismatch!: pw:%s c:%02X[%c] node.f8:[%02X] "
      //            "bw.f8:[%02X] bw->fw.f8:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.f8, n.digits.f8,
      //            n2.digits.f8);
      //   }
      //   if (node.digits.f9 != n2.digits.f9) {
      //     printf("F9 mismatch!: pw:%s c:%02X[%c] node.f9:[%02X] "
      //            "bw.f9:[%02X] bw->fw.f9:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.f9, n.digits.f9,
      //            n2.digits.f9);
      //   }
      //   if (node.digits.fa != n2.digits.fa) {
      //     printf("FA mismatch!: pw:%s c:%02X[%c] node.fa:[%02X] "
      //            "bw.fa:[%02X] bw->fw.fa:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.fa, n.digits.fa,
      //            n2.digits.fa);
      //   }
      //   if (node.digits.fb != n2.digits.fb) {
      //     printf("FB mismatch!: pw:%s c:%02X[%c] node.fb:[%02X] "
      //            "bw.fb:[%02X] bw->fw.fb:[%02X]\n",
      //            node.pw.c_str(), c, atoy[c], node.digits.fb, n.digits.fb,
      //            n2.digits.fb);
      //   }
      // }
      // std::vector<Node> new_nodes = backward_step_simd(node);
      pool.insert(pool.end(), new_nodes.begin(), new_nodes.end());
    }
  }

  char minmax_filename[256];
  snprintf(minmax_filename, 256, "%s/minmax.dat", basename);
  FILE *fp = fopen(minmax_filename, "wb");
  if (!fp) {
    printf("minmax.dat open error!.");
  } else {
    unsigned char minmax[2];
    minmax[0] = fbmin;
    minmax[1] = fbmax;
    fwrite(minmax, 2, 1, fp);
    fclose(fp);
  }
  printf("End, count: %llu found_count: %llu\n", count, found_count);

  printf("結果ソート中\n");
  std::sort(bwi_vector.begin(), bwi_vector.end(), bwi_comp);

  printf("結果書き出し中\n");
  char filename[256];
  snprintf(filename, 256, "%s/backward.dat", basename);

  fp = fopen(filename, "wb");
  fwrite(&bwi_vector[0], sizeof(BackwardInfo), bwi_vector.size(), fp);
  fclose(fp);

  printf("完了\n");
}
