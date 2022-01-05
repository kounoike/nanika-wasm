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
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <bzlib.h>

using std::vector;

const int PWLEN_MAX = 16;
const int NUM_CHARSET_nmc = 42;
const int NUM_CHARSET = 39;
const int BULK_SIZE = 64;

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

struct PackedInfo {
  unsigned char f7;
  unsigned char f8;
  unsigned char f9;
  unsigned char fa;
  unsigned char fb;
  char pw[PWLEN_MAX];
};

bool pi_comp(const PackedInfo &a, const PackedInfo &b) {
  if (a.f7 != b.f7)
    return a.f7 < b.f7;
  else if (a.f8 != b.f8)
    return a.f8 < b.f8;
  else if (a.f9 != b.f9)
    return a.f9 < b.f9;
  else if (a.fa != b.fa)
    return a.fa < b.fa;
  else
    return a.fb < b.fb;
};

struct ThreadInfo {
  int thread_id;
  bool is_finished;
  Node start_node;
  unsigned long long found_count;
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

std::vector<Node> backward_step(const Node &node, unsigned char p) {
  const Digits &next = node.digits;
  Digits prev;

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

    ret.push_back(std::move(new_node));
  }

  return ret;
}

size_t get_index(unsigned int f4, unsigned int f5) {
  return ((f4 & 0xff) << 8) | (f5 & 0xff);
}

void calc_forward_thread(ThreadInfo &ti, std::vector<std::mutex> &forward_mtx,
                         int atk_count, int fbmin, int fbmax, int backward_len,
                         int atk31F7, int atk31F9, int atk31FB,
                         std::vector<BZFILE *> &bzfp_vector,
                         std::vector<unsigned long long> &count_vector) {
  std::vector<Node> pool;
  pool.push_back(ti.start_node);

  if (ti.thread_id == 0) {
    printf("順探索スレッドID0起動\n");
  }

  // ロックが取れないときに待つのが無駄なのでキャッシュする
  // そのためのキャッシュ
  std::vector<std::vector<PackedInfo>> cache(256 * 256);

  // 探索
  unsigned long long count = 0;
  unsigned long long found_count = 0;
  while (!pool.empty()) {
    count++;
    Node node = pool.back();
    pool.pop_back();

    // 目標深さに到達
    if (node.depth >= atk_count - backward_len) {
      ti.found_count++;
      Digits &d = node.digits;
      PackedInfo info;
      info.f7 = d.f7;
      info.f8 = d.f8;
      info.f9 = d.f9;
      info.fa = d.fa;
      info.fb = d.fb;
      for (int i = 0; i < node.depth; i++) {
        info.pw[i] = node.pw[i];
      }
      // memcpy(info.pw, node.pw.c_str(), node.depth);
      size_t idx = get_index(d.f4, d.f5);

      // ロックが取れないときに待つのが無駄なのでキャッシュする
      // キャッシュに溜まってる分もついでに書き出すのでまずキャッシュに追加
      cache[idx].push_back(info);
      if (forward_mtx[idx].try_lock()) {
        int bzError;
        BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                    sizeof(PackedInfo) * cache[idx].size());
        count_vector[idx] += cache[idx].size();
        cache[idx].clear();
        forward_mtx[idx].unlock();
      }

    } else {
      // SIMD版
      std::vector<Node> new_nodes = forward_step_simd(node);
      pool.insert(pool.end(), new_nodes.begin(), new_nodes.end());

      // 通常版
      // for (int i = 0; i < NUM_CHARSET; i++) {
      //   unsigned char p = itoa[i];
      //   Node n = forward_step(node, p);
      //   pool.push_back(n);
      // }
    }
  }
  if (ti.thread_id == 0) {
    printf("順探索スレッドID0 探索終了。書き出し\n");
  }

  // まずはロックが取れたものだけ書き出す
  for (int idx = 0; idx < 256 * 256; idx++) {
    if (!cache[idx].empty()) {
      if (forward_mtx[idx].try_lock()) {
        int bzError;
        BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                    sizeof(PackedInfo) * cache[idx].size());
        count_vector[idx] += cache[idx].size();
        cache[idx].clear();
        forward_mtx[idx].unlock();
      }
    }
  }

  // 2回目は仕方ないからロックが取れるまで待って書き出す
  for (int idx = 0; idx < 256 * 256; idx++) {
    if (!cache[idx].empty()) {
      std::lock_guard<std::mutex> lock(forward_mtx[idx]);
      int bzError;
      BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                  sizeof(PackedInfo) * cache[idx].size());
      count_vector[idx] += cache[idx].size();
      cache[idx].clear();
    }
  }

  if (ti.thread_id == 0) {
    printf("順探索スレッドID0終了\n");
  }

  ti.is_finished = true;
}

void calc_backward_thread(ThreadInfo &ti, std::vector<std::mutex> &backward_mtx,
                          int atk_count, int fbmin, int fbmax, int backward_len,
                          int atk31F7, int atk31F9, int atk31FB,
                          std::vector<BZFILE *> &bzfp_vector,
                          std::vector<unsigned long long> &count_vector) {
  //
  std::vector<Node> pool;
  pool.push_back(std::move(ti.start_node));

  // 探索
  unsigned long long count = 0;
  unsigned long long found_count = 0;

  // ロックが取れないときに待つのが無駄なのでキャッシュする
  // そのためのキャッシュ
  std::vector<std::vector<PackedInfo>> cache(256 * 256);

  while (!pool.empty()) {
    count++;
    Node node = pool.back();
    pool.pop_back();

    // 目標深さに到達
    if (node.depth >= backward_len) {
      Digits &d = node.digits;
      PackedInfo info;
      info.f7 = d.f7;
      info.f8 = d.f8;
      info.f9 = d.f9;
      info.fa = d.fa;
      info.fb = d.fb;
      memcpy(info.pw, node.pw.c_str(), node.depth);
      size_t idx = get_index(d.f4, d.f5);
      // ロックが取れないときに待つのが無駄なのでキャッシュする
      // キャッシュに溜まってる分もついでに書き出すのでまずキャッシュに追加
      cache[idx].push_back(info);
      if (backward_mtx[idx].try_lock()) {
        int bzError;
        BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                    sizeof(PackedInfo) * cache[idx].size());
        count_vector[idx] += cache[idx].size();
        cache[idx].clear();
        backward_mtx[idx].unlock();
      }

      found_count++;
      continue;
    }

    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char c = itoa[i];
      std::vector<Node> new_nodes = backward_step(node, c);
      pool.insert(pool.end(), new_nodes.begin(), new_nodes.end());
    }
  }

  // まずはロックが取れたものだけ書き出す
  for (int idx = 0; idx < 256 * 256; idx++) {
    if (!cache[idx].empty()) {
      if (backward_mtx[idx].try_lock()) {
        int bzError;
        BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                    sizeof(PackedInfo) * cache[idx].size());
        count_vector[idx] += cache[idx].size();
        cache[idx].clear();
        backward_mtx[idx].unlock();
      }
    }
  }

  // 2回目は仕方ないからロックが取れるまで待って書き出す
  for (int idx = 0; idx < 256 * 256; idx++) {
    if (!cache[idx].empty()) {
      std::lock_guard<std::mutex> lock(backward_mtx[idx]);
      int bzError;
      BZ2_bzWrite(&bzError, bzfp_vector[idx], &cache[idx][0],
                  sizeof(PackedInfo) * cache[idx].size());
      count_vector[idx] += cache[idx].size();
      cache[idx].clear();
    }
  }

  ti.is_finished = true;
  ti.found_count = found_count;
}

int main(int argc, char *argv[]) {
  create_lut();
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

  int backward_len = atk_count / 2;
  printf("Backward_len: %d\n", backward_len);

  char basepart[40];
  snprintf(basepart, 40, "dat_%02X%02X%02X%02X%02X%02X%02X%02X", atk31F4,
           atk31F5, atk_count, atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);

  if (std::filesystem::exists(basepart)) {
    printf("%s: 既に作成済みです。再生成するときは一度消してください。\n",
           basepart);
    return 0;
  }

  std::filesystem::create_directories(std::string(basepart) + "/fw");
  std::filesystem::create_directories(std::string(basepart) + "/bw");

  // 14桁パスワード特化
  // int fbmin = 1; // bw6段探索結果より
  // int fbmax = 5 + 0x1a; // bw6段探索結果より
  // 最終6段で0x1a、残り1段は大きく見て5
  // 一般
  int fbmin = 0;
  int fbmax = 5 * backward_len;

  int cpu_count = std::thread::hardware_concurrency();

  // 順探索スコープ
  {
    // 最初を入れる
    Node forward_start_node;
    forward_start_node.depth = 0;
    forward_start_node.pw = std::string("");
    forward_start_node.digits.f4 = 0;
    forward_start_node.digits.f5 = 0;
    forward_start_node.digits.f6 = atk_count;
    forward_start_node.digits.f7 = 0;
    forward_start_node.digits.f8 = 0;
    forward_start_node.digits.f9 = 0;
    forward_start_node.digits.fa = 1;
    forward_start_node.digits.fb = 0;

    // プレフィックス対応
    for (int i = 0; i < prefix_count; i++) {
      unsigned char p = prefix[i];
      forward_start_node = forward_step(forward_start_node, p);
    }

    printf("順探索スレッド初期データ生成中...\n");
    unsigned long long found_count = 0;
    std::vector<ThreadInfo> thread_info_vector;
    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char p1 = itoa[i];
      Node node1 = forward_step(forward_start_node, p1);
      for (int j = 0; j < NUM_CHARSET; j++) {
        unsigned char p2 = itoa[j];
        Node node2 = forward_step(node1, p2);
        ThreadInfo ti;
        ti.is_finished = false;
        ti.start_node = node2;
        ti.thread_id = i * NUM_CHARSET + j;
        ti.found_count = 0;
        thread_info_vector.push_back(std::move(ti));
      }
    }
    printf("...完了\n");

    int create_count = 0;
    std::vector<std::thread> threads;
    std::vector<std::mutex> forward_mtx(256 * 256);
    std::vector<FILE *> fp_vector(256 * 256, NULL);
    std::vector<BZFILE *> bzfp_vector(256 * 256, NULL);
    std::vector<unsigned long long> count_vector(256 * 256, 0);
    std::vector<unsigned long long> found_counts(cpu_count, 0);

    printf("BZFILEオープン...\n");
    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        size_t idx = get_index(i, j);
        char filename[256];
        snprintf(filename, 256, "%s/fw/%02X%02X.dat", basepart, i, j);
        fp_vector[idx] = fopen(filename, "wb");
        assert(fp_vector[idx]);

        int bzError;
        bzfp_vector[idx] = BZ2_bzWriteOpen(&bzError, fp_vector[idx], 7, 0, 0);
        assert(bzError == BZ_OK);
      }
    }
    printf("...完了\n");

    printf("順探索マルチスレッド探索開始...\n");

    for (int id = 0; id < cpu_count; id++) {
      threads.push_back(std::thread([&, id]() {
        for (int i = id; i < thread_info_vector.size(); i += cpu_count) {
          calc_forward_thread(thread_info_vector[i], forward_mtx, atk_count,
                              fbmin, fbmax, backward_len, atk31F7, atk31F9,
                              atk31FB, bzfp_vector, count_vector);
          found_counts[id] += thread_info_vector[i].found_count;
          if (id == 0) {
            printf("順探索[%d] 探索終了\n", i);
          }
        }
      }));
    }
    printf("スレッド終了待ち\n");
    for (int id = 0; id < cpu_count; id++) {
      threads[id].join();
      found_count += found_counts[id];
    }
    printf("ファイルクローズ...\n");
    std::vector<std::thread> close_threads;
    for (int i = 0; i < 256; i++) {
      close_threads.push_back(std::thread([&, i]() {
        for (int j = 0; j < 256; j++) {
          size_t idx = get_index(i, j);
          int bzError;
          BZ2_bzWriteClose(&bzError, bzfp_vector[idx], 0, NULL, NULL);
          assert(bzError != BZ_IO_ERROR);
          assert(bzError == BZ_OK);
          fclose(fp_vector[idx]);
          char filename[256];
          snprintf(filename, 256, "%s/fw/%02X%02X.cnt", basepart, i, j);
          FILE *fp = fopen(filename, "wb");
          fwrite(&count_vector[idx], sizeof(unsigned long long), 1, fp);
          fclose(fp);
        }
      }));
    }
    for (auto &th : close_threads) {
      th.join();
    }
    printf("...完了\n");
  }

  // 逆探索スコープ
  {
    printf("逆探索スレッド初期データ作成開始...\n");
    // 最初を入れる
    Node backward_start_node;
    backward_start_node.depth = 0;
    backward_start_node.pw = std::string("");
    backward_start_node.digits.f4 = atk31F4;
    backward_start_node.digits.f5 = atk31F5;
    backward_start_node.digits.f6 = atk_count;
    backward_start_node.digits.f7 = atk31F7;
    backward_start_node.digits.f8 = atk31F8;
    backward_start_node.digits.f9 = atk31F9;
    backward_start_node.digits.fa = atk31FA;
    backward_start_node.digits.fb = atk31FB;
    unsigned long long found_count = 0;
    std::vector<ThreadInfo> thread_info_vector;
    for (int i = 0; i < NUM_CHARSET; i++) {
      unsigned char p1 = itoa[i];
      vector<Node> nodes1 = backward_step(backward_start_node, p1);
      for (auto node1 : nodes1) {
        for (int j = 0; j < NUM_CHARSET; j++) {
          unsigned char p2 = itoa[j];
          vector<Node> nodes2 = backward_step(node1, p2);
          for (auto node2 : nodes2) {
            ThreadInfo ti;
            ti.is_finished = false;
            ti.start_node = node2;
            ti.thread_id = i * NUM_CHARSET + j;
            ti.found_count = 0;
            thread_info_vector.push_back(std::move(ti));
          }
        }
      }
    }
    printf("...完了\n");

    printf("逆探索マルチスレッド探索開始...\n");
    int create_count = 0;
    int cpu_count = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<int> current_targets;
    std::vector<unsigned long long> found_counts(cpu_count, 0);
    std::vector<std::mutex> backward_mtx(256 * 256);
    std::vector<FILE *> fp_vector(256 * 256, NULL);
    std::vector<BZFILE *> bzfp_vector(256 * 256, NULL);
    std::vector<unsigned long long> count_vector(256 * 256, 0);

    printf("BZFILEオープン...\n");
    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        size_t idx = get_index(i, j);
        char filename[256];
        snprintf(filename, 256, "%s/bw/%02X%02X.dat", basepart, i, j);
        fp_vector[idx] = fopen(filename, "wb");
        assert(fp_vector[idx]);

        int bzError;
        bzfp_vector[idx] = BZ2_bzWriteOpen(&bzError, fp_vector[idx], 7, 0, 0);
        assert(bzError == BZ_OK);
      }
    }
    printf("...完了\n");

    for (int id = 0; id < cpu_count; id++) {
      threads.push_back(std::thread([&, id]() {
        for (int i = id; i < thread_info_vector.size(); i += cpu_count) {
          calc_backward_thread(thread_info_vector[i], backward_mtx, atk_count,
                               fbmin, fbmax, backward_len, atk31F7, atk31F9,
                               atk31FB, bzfp_vector, count_vector);
          found_counts[id] += thread_info_vector[i].found_count;
          if (id == 0) {
            printf("逆探索[%d] 探索終了\n", i);
          }
        }
      }));
    }
    for (int id = 0; id < cpu_count; id++) {
      threads[id].join();
      found_count += found_counts[id];
    }
    printf("逆探索完了\n");

    printf("ファイルクローズ...\n");
    std::vector<std::thread> close_threads;
    for (int i = 0; i < 256; i++) {
      close_threads.push_back(std::thread([&, i]() {
        for (int j = 0; j < 256; j++) {
          size_t idx = get_index(i, j);
          int bzError;
          BZ2_bzWriteClose(&bzError, bzfp_vector[idx], 0, NULL, NULL);
          assert(bzError != BZ_IO_ERROR);
          assert(bzError == BZ_OK);
          fclose(fp_vector[idx]);
          char filename[256];
          snprintf(filename, 256, "%s/bw/%02X%02X.cnt", basepart, i, j);
          FILE *fp = fopen(filename, "wb");
          fwrite(&count_vector[idx], sizeof(unsigned long long), 1, fp);
          fclose(fp);
        }
      }));
    }
    for (auto &th : close_threads) {
      th.join();
    }
    printf("...完了\n");
  }

  // 結果照合
  printf("結果照合処理開始\n");
  {
    std::vector<std::thread> threads;
    std::mutex mtx;

    char result_filename[256];
    snprintf(result_filename, 256, "%s/result.txt", basepart);
    FILE *result_fp = fopen(result_filename, "w");

    std::vector<std::function<void()>> fwreader, bwreader;
    std::vector<std::vector<PackedInfo>> fwinfo_vector(
        256 * 256, std::vector<PackedInfo>());
    std::vector<std::vector<PackedInfo>> bwinfo_vector(
        256 * 256, std::vector<PackedInfo>());
    std::vector<std::thread> fwread_threads(256 * 256),
        bwread_threads(256 * 256);

    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        size_t idx = get_index(i, j);
        fwreader.push_back([&, i, j, idx]() {
          char fwfilename[256];
          snprintf(fwfilename, 256, "%s/fw/%02X%02X.dat", basepart, i, j);
          char fwcntfilename[256];
          snprintf(fwcntfilename, 256, "%s/fw/%02X%02X.cnt", basepart, i, j);
          if (!std::filesystem::exists(fwfilename)) {
            return;
          }
          unsigned long long fwcount;
          size_t ret;
          FILE *fwcnt_fp = fopen(fwcntfilename, "rb");
          ret = fread(&fwcount, sizeof(unsigned long long), 1, fwcnt_fp);
          assert(ret == 1);
          fclose(fwcnt_fp);
          if (fwcount == 0) {
            return;
          }
          std::vector<PackedInfo> fwinfo(fwcount);

          FILE *fwfp = fopen(fwfilename, "rb");
          assert(fwfp);
          int bzError;
          int read_count = 0;
          BZFILE *fwbzfp = BZ2_bzReadOpen(&bzError, fwfp, 0, 0, NULL, 0);
          assert(bzError == BZ_OK);
          BZ2_bzRead(&bzError, fwbzfp, &fwinfo[0],
                     sizeof(PackedInfo) * fwcount);
          assert(bzError == BZ_OK || bzError == BZ_STREAM_END);
          BZ2_bzReadClose(&bzError, fwbzfp);
          assert(bzError == BZ_OK);
          fclose(fwfp);
          fwinfo.swap(fwinfo_vector[idx]);
        });

        bwreader.push_back([&, i, j, idx]() {
          char bwfilename[256];
          snprintf(bwfilename, 256, "%s/bw/%02X%02X.dat", basepart, i, j);
          char bwcntfilename[256];
          snprintf(bwcntfilename, 256, "%s/bw/%02X%02X.cnt", basepart, i, j);
          if (!std::filesystem::exists(bwfilename)) {
            return;
          }
          unsigned long long bwcount;
          size_t ret;
          FILE *bwcnt_fp = fopen(bwcntfilename, "rb");
          ret = fread(&bwcount, sizeof(unsigned long long), 1, bwcnt_fp);
          assert(ret == 1);
          fclose(bwcnt_fp);
          if (bwcount == 0) {
            return;
          }
          std::vector<PackedInfo> bwinfo(bwcount);

          FILE *bwfp = fopen(bwfilename, "rb");
          assert(bwfp);
          int bzError;
          int read_count = 0;
          BZFILE *bwbzfp = BZ2_bzReadOpen(&bzError, bwfp, 0, 0, NULL, 0);
          BZ2_bzRead(&bzError, bwbzfp, &bwinfo[0],
                     sizeof(PackedInfo) * bwcount);
          BZ2_bzReadClose(&bzError, bwbzfp);
          fclose(bwfp);
          bwinfo.swap(bwinfo_vector[idx]);
        });
      }
    }

    std::vector<std::thread> matching_threads;
    int matching_parallel = 32;
    for (int mid = 0; mid < matching_parallel; mid++) {
      fwread_threads[mid] = std::thread(fwreader[mid]);
      bwread_threads[mid] = std::thread(bwreader[mid]);
      if (mid + matching_parallel < 256 * 256) {
        fwread_threads[mid + matching_parallel] =
            std::thread(fwreader[mid + matching_parallel]);
        bwread_threads[mid + matching_parallel] =
            std::thread(bwreader[mid + matching_parallel]);
      }

      matching_threads.push_back(std::thread([&, mid]() {
        for (int i = mid; i < 256 * 256; i += matching_parallel) {
          if (mid == 0)
            printf("照合[%d]開始\n", i);
          // マージ処理の前でBZIP読み込みスレッドを開始しておく
          if (i + matching_parallel * 2 < 256 * 256) {
            fwread_threads[i + matching_parallel * 2] =
                std::thread(fwreader[i + matching_parallel * 2]);
            bwread_threads[i + matching_parallel * 2] =
                std::thread(bwreader[i + matching_parallel * 2]);
          }

          fwread_threads[i].join();
          bwread_threads[i].join();

          std::vector<PackedInfo> &fwinfo = fwinfo_vector[i];
          std::vector<PackedInfo> &bwinfo = bwinfo_vector[i];

          if (mid == 0)
            printf(
                "[%d] bzip読み込み待機完了 fwinfo.size:%lu bwinfo.size:%lu\n",
                i, fwinfo.size(), bwinfo.size());

          std::vector<std::thread> threads;
          // ソートを並列化する
          int block_count = 8;

          for (int id = 0; id < block_count; id++) {
            threads.push_back(std::thread([&, id]() {
              auto fwbegin =
                  fwinfo.begin() + (fwinfo.size() / block_count) * id;
              auto fwend = fwbegin + (fwinfo.size() / block_count);
              if (fwend > fwinfo.end()) {
                fwend = fwinfo.end();
              }
              std::sort(fwbegin, fwend, pi_comp);

              auto bwbegin =
                  bwinfo.begin() + (bwinfo.size() / block_count) * id;
              auto bwend = bwbegin + (bwinfo.size() / block_count);
              if (bwend > bwinfo.end()) {
                bwend = bwinfo.end();
              }
              std::sort(bwbegin, bwend, pi_comp);
            }));
          }
          for (int id = 0; id < block_count; id++) {
            threads[id].join();
          }

          // マージ用ブロック
          if (mid == 0)
            printf("[%d]マージ処理開始\n", i);
          std::thread fwmerge_thread = std::thread([&]() {
            std::vector<PackedInfo> tmp, fwsorted;

            for (int id = 0; id < block_count; id++) {
              auto fwbegin =
                  fwinfo.begin() + (fwinfo.size() / block_count) * id;
              auto fwend = fwbegin + (fwinfo.size() / block_count);
              if (fwend > fwinfo.end()) {
                fwend = fwinfo.end();
              }
              fwsorted.swap(tmp);
              std::merge(fwbegin, fwend, tmp.begin(), tmp.end(),
                         std::back_inserter(fwsorted), pi_comp);
              tmp.clear();
            }
            fwinfo.swap(fwsorted);
          });
          std::thread bwmerge_thread = std::thread([&]() {
            std::vector<PackedInfo> tmp, bwsorted;

            for (int id = 0; id < block_count; id++) {
              auto bwbegin =
                  bwinfo.begin() + (bwinfo.size() / block_count) * id;
              auto bwend = bwbegin + (bwinfo.size() / block_count);
              if (bwend > bwinfo.end()) {
                bwend = bwinfo.end();
              }
              bwsorted.swap(tmp);
              std::merge(bwbegin, bwend, tmp.begin(), tmp.end(),
                         std::back_inserter(bwsorted), pi_comp);
              tmp.clear();
            }
            bwinfo.swap(bwsorted);
          });
          fwmerge_thread.join();
          bwmerge_thread.join();
          if (mid == 0)
            printf("[%d]マージ処理終了\n", i);

          if (mid == 0)
            printf("[%d]intersection処理開始\n", i);
          std::vector<PackedInfo> intersection;
          std::set_intersection(fwinfo.begin(), fwinfo.end(), bwinfo.begin(),
                                bwinfo.end(), std::back_inserter(intersection),
                                pi_comp);

          if (mid == 0)
            printf("[%d]列挙処理開始 fw.size:%lu, bw.size:%lu "
                   "intersection.size:%lu\n",
                   i, fwinfo.size(), bwinfo.size(), intersection.size());
          for (auto &ref : intersection) {
            auto fwp =
                std::equal_range(fwinfo.begin(), fwinfo.end(), ref, pi_comp);
            auto bwp =
                std::equal_range(bwinfo.begin(), bwinfo.end(), ref, pi_comp);

            for (auto fwit = fwp.first; fwit != fwp.second; ++fwit) {
              for (auto bwit = bwp.first; bwit != bwp.second; ++bwit) {
                char pw[PWLEN_MAX + 1];
                memcpy(pw, fwit->pw, atk_count - backward_len);
                memcpy(&pw[atk_count - backward_len], bwit->pw, backward_len);
                pw[atk_count] = '\n';
                pw[atk_count + 1] = '\0';

                {
                  std::lock_guard<std::mutex> lock(mtx);
                  // printf("Hit: %s", pw);
                  fputs(pw, result_fp);
                }
              }
            }
          }

          if (mid == 0)
            printf("照合[%d] 照合終了\n", i);

          std::vector<PackedInfo> dummy0(0), dummy1(0);
          fwinfo_vector[i].swap(dummy0);
          bwinfo_vector[i].swap(dummy1);
        }
      }));
    }

    printf("照合処理スレッド終了待ち\n");
    for (int mid = 0; mid < matching_parallel; mid++) {
      matching_threads[mid].join();
    }

    fclose(result_fp);
  }
  printf("結果照合処理終了\n");

  // fclose(result_fp);
  return 0;
}
