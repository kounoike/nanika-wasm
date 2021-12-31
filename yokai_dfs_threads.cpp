// yokai03mc.cpp : このファイルには 'main'
// 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <bit>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include <immintrin.h>

#define BULK_SIZE 64
#define PWLEN_MAX 16

// 文字コード変換テーブル
unsigned char atoy[256] = {
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

unsigned char itoa[42] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0a,
                          0x0b, 0x0c, 0x0d, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                          0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x20, 0x21, 0x22,
                          0x23, 0x24, 0x25, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
                          0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

struct BulkCheckDigits {
  alignas(64) unsigned char F4[BULK_SIZE];
  alignas(64) unsigned char F5[BULK_SIZE];
  alignas(64) unsigned char F7[BULK_SIZE];
  alignas(64) unsigned char F8[BULK_SIZE];
  alignas(64) unsigned char F9[BULK_SIZE];
  alignas(64) unsigned char FA[BULK_SIZE];
  alignas(64) unsigned char FB[BULK_SIZE];
};

struct DfsNode {
  int depth;
  unsigned char PW[PWLEN_MAX];
};

struct ThreadData {
  int threadId;
  bool isFinished;
  DfsNode startNode;
  BulkCheckDigits threadCheckDigits[PWLEN_MAX];
  unsigned char PW1;
  unsigned char PW2;
};

alignas(64) const static unsigned char PopCount[BULK_SIZE] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3,
    3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,
    3, 4, 4, 5, 2, 3, 3, 4, 5, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
};

void calc_next(const BulkCheckDigits &prevCD, BulkCheckDigits &nextCD,
               unsigned char prevChar) {
  alignas(64) unsigned char PW[BULK_SIZE];
  alignas(64) unsigned char A[BULK_SIZE];
  alignas(64) unsigned char A1[BULK_SIZE];
  alignas(64) unsigned char C[BULK_SIZE];
  alignas(64) unsigned char C1[BULK_SIZE];

  int Y;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    PW[idx] = idx;

  // $31F4, $31F5
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.F4[idx] = prevCD.F4[prevChar];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.F5[idx] = prevCD.F5[prevChar];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = PW[idx];

  for (int Y = 0; Y < 8; Y++) {
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = A[idx] & 0x80;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A[idx] <<= 1;

    for (int idx = 0; idx < BULK_SIZE; idx++)
      C1[idx] = nextCD.F4[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      nextCD.F4[idx] = (nextCD.F4[idx] >> 1) | C[idx];
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = nextCD.F5[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      nextCD.F5[idx] = (nextCD.F5[idx] >> 1) | (C1[idx] << 7);

    for (int idx = 0; idx < BULK_SIZE; idx++)
      nextCD.F4[idx] ^= C[idx] > 0 ? 0x84 : 0;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      nextCD.F5[idx] ^= C[idx] > 0 ? 0x08 : 0;
  }

  // $31F7, $31F8
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = nextCD.F4[idx] >= 0xE5 ? 1 : 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.F7[idx] = PW[idx] + prevCD.F7[prevChar] + C[idx];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    // ADCのキャリー処理
    C[idx] =
        (nextCD.F7[idx] < PW[idx] || (C[idx] > 0 && nextCD.F7[idx] == PW[idx]))
            ? 1
            : 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.F8[idx] = prevCD.F8[prevChar] + nextCD.F5[idx] + C[idx];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] = (nextCD.F8[idx] < nextCD.F5[idx] ||
              (C[idx] > 0 && nextCD.F8[idx] == nextCD.F5[idx]))
                 ? 1
                 : 0; // ADCのキャリー処理

  // $31F9 (xor)
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.F9[idx] = prevCD.F9[prevChar] ^ PW[idx];

  // $31FA
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C1[idx] = prevCD.FA[prevChar] & 0x01;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.FA[idx] =
        prevCD.FA[prevChar] >> 1 | (C[idx] << 7); // $31F8のCがここで入る
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.FA[idx] = PW[idx] + nextCD.FA[idx] + C1[idx];
  for (int idx = 0; idx < BULK_SIZE; idx++)
    C[idx] =
        (nextCD.FA[idx] < PW[idx] || (C1[idx] > 0 && nextCD.FA[idx] == PW[idx]))
            ? 1
            : 0; // ADCのキャリー処理
  // $31FB
  // popcount計算1: std::popcountを使う
  // for (int idx = 0; idx < BULK_SIZE; idx++)
  //   A[idx] = std::popcount(PW[idx]);
  // for (int idx = 0; idx < BULK_SIZE; idx++)
  //   nextCD.FB[idx] = prevCD.FB[prevChar] + A[idx] + C[idx];

  // popcount計算2: 6bit(文字コードの関係)分回す
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A[idx] = 0;
  for (int idx = 0; idx < BULK_SIZE; idx++)
    A1[idx] = PW[idx];
  for (Y = 0; Y < 6; Y++) {
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A[idx] += A1[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A1[idx] >>= 1;
  }
  for (int idx = 0; idx < BULK_SIZE; idx++)
    nextCD.FB[idx] = prevCD.FB[prevChar] + A[idx] + C[idx];

  // popcount計算3: 事前計算済みの値
  // for (int idx = 0; idx < BULK_SIZE; idx++)
  //   nextCD.FB[idx] = prevCD.FB[prevChar] + PopCount[idx] + C[idx];
}

std::string pw_to_string(unsigned char *p, int len) {
  std::string s = "";
  for (int i = 0; i < len; i++) {
    s += atoy[p[i]];
  }
  return s;
}

void printCheckDigits(const BulkCheckDigits &cd, unsigned char c) {
  printf("$31F4:[%02X] $31F5:[%02X] $31F7:[%02X] $31F8:[%02X] $31F9:[%02X] "
         "$31FA:[%02X] $31FB:[%02X]\n",
         cd.F4[c], cd.F5[c], cd.F7[c], cd.F8[c], cd.F9[c], cd.FA[c], cd.FB[c]);
}

void calc_thread(ThreadData &thread_data, std::mutex &mtx, int atk_count,
                 int atk31F4, int atk31F5, int atk31F7, int atk31F8,
                 int atk31F9, int atk31FA, int atk31FB) {
  // スレッドの本体
  {
    std::lock_guard<std::mutex> lock(mtx);
    printf("thread[%d]: depth:%d PW1:%c PW2:%c PW[%c%c] f4:[%02X] f5:[%02X] "
           "f7:[%02X] f8:[%02X] f9:[%02X] fa:[%02X] fb:[%02X]\n",
           thread_data.threadId, thread_data.startNode.depth,
           atoy[thread_data.PW1], atoy[thread_data.PW2],
           atoy[thread_data.startNode.PW[thread_data.startNode.depth - 2]],
           atoy[thread_data.startNode.PW[thread_data.startNode.depth - 1]],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .F4[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .F5[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .F7[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .F8[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .F9[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .FA[thread_data.PW2],
           thread_data.threadCheckDigits[thread_data.startNode.depth]
               .FB[thread_data.PW2]);
  }
  std::vector<DfsNode> pool;
  pool.push_back(std::move(thread_data.startNode));

  unsigned long long count = 0;
  while (!pool.empty()) {
    count++;
    DfsNode cur = pool.back();
    pool.pop_back();

    BulkCheckDigits &cd = thread_data.threadCheckDigits[cur.depth];
    int rem_chars = atk_count - cur.depth;

    // Debug
    // if (count == 10) {
    //   std::lock_guard<std::mutex> lock(mtx);
    //   printf("count: %llu, cur.depth: %d cur.PW: %02X %02X %02X.. : %s
    //   f4:%02X "
    //          "f5:%02X f7:%02X f8:%02X f9:%02X fa:%02X fb:%02X\n",
    //          count, cur.depth, cur.PW[0], cur.PW[1], cur.PW[2],
    //          pw_to_string(cur.PW, cur.depth).c_str(),
    //          cd.F4[cur.PW[cur.depth - 1]], cd.F5[cur.PW[cur.depth - 1]],
    //          cd.F7[cur.PW[cur.depth - 1]], cd.F8[cur.PW[cur.depth - 1]],
    //          cd.F9[cur.PW[cur.depth - 1]], cd.FA[cur.PW[cur.depth - 1]],
    //          cd.FB[cur.PW[cur.depth - 1]]);
    // }

    // $31FBは文字コードのビットが1になってるものの総和＋キャリーの和
    // 文字コード集合の関係で1文字で4、キャリー足して5が最大
    // 256を超えることはないので減ることはない
    unsigned char fb = cd.FB[cur.PW[cur.depth - 1]];
    if (fb > atk31FB || fb + 5 * rem_chars < atk31FB) {
      continue;
    }
    if (rem_chars > 0 && rem_chars <= 4) {
      // $31F7は文字コードの総和＋キャリーの和（255を超えてループすることもある）
      // 文字コードは0x35が最大、キャリーは0か1なので、1文字あたり最大で0x35 + 1
      // = 54までしか増えない 最大まで増やしても目標値に到達しなければ枝狩り可能
      // 5文字あると54 * 5 = 270で一周してしまうので、4文字以内のときだけ使える
      unsigned int f7 = cd.F7[cur.PW[cur.depth - 1]];
      unsigned int target = atk31F7;
      if (target < f7) {
        // 一周してくる必要がある
        target += 256;
      }
      // last_carry:
      // 最後の桁の$31F4は既知なのでキャリーが発生する(>=0xE5)かどうか事前に分かる
      int last_carry = atk31F4 >= 0xE5 ? 1 : 0;
      // $31F9: パスワード全体のxor
      unsigned char f9 = cd.F9[cur.PW[cur.depth - 1]];
      // f9 ^ atk31F9
      // の各ビットで、「残りの文字でそのビットを何回使うか」の偶奇が分かる
      if (((f9 ^ atk31F9) & 0x20) >> 5 != rem_chars % 2) {
        // 0x20のビットを全部では使えない。1回は最大値が0x1Dになる
        if (f7 + (rem_chars - 1) * (0x35 + 1) + 0x1D + last_carry < target) {
          continue;
        }
      } else if (((f9 ^ atk31F9) & 0x10) >> 4 != rem_chars % 2) {
        // 0x10のビットを全部では使えない。1回は最大値が0x2Dになる
        if (f7 + (rem_chars - 1) * (0x35 + 1) + 0x2D + last_carry < target) {
          continue;
        }
      } else {
        if (f7 + rem_chars * (0x35 + 1) - 1 + last_carry < target) {
          continue;
        }
      }
    }

    if (cur.depth == atk_count - 2) {
      // $31F4と$31F5の下2ビットにパスワード後ろ2文字は影響を及ぼさない
      // 解説: https://gist.github.com/kounoike/340c1dbf684bc2cc5dd0169b1f824317
      // Y(14,1) = popcount(((Y(12) & 0b01000110) << 8) | (Z(12) & 0b01101110))
      // % 2 Y(14,0) = popcount(((Y(12) & 0b00100011) << 8) | (Z(12) &
      // 0b00110111)) % 2

      // Z(14,1) = popcount(((Y(12) & 0b00100010) << 8) | (Z(12) & 0b01100000))
      // % 2 Z(14,0) = popcount(((Y(12) & 0b00010001) << 8) | (Z(12) &
      // 0b00110000)) % 2

      BulkCheckDigits &cd = thread_data.threadCheckDigits[cur.depth];
      unsigned int f4 = cd.F4[cur.PW[cur.depth - 1]];
      unsigned int f5 = cd.F5[cur.PW[cur.depth - 1]];
      unsigned int f4_1_bits = ((f4 & 0b01000110) << 8) | (f5 & 0b01101110);
      unsigned int f4_0_bits = ((f4 & 0b00100011) << 8) | (f5 & 0b00110111);
      unsigned int f5_1_bits = ((f4 & 0b00100010) << 8) | (f5 & 0b01100000);
      unsigned int f5_0_bits = ((f4 & 0b00010001) << 8) | (f5 & 0b00110000);
      if (((std::popcount(f4_1_bits) & 0x01) != ((atk31F4 >> 1) & 0x01)) ||
          ((std::popcount(f4_0_bits) & 0x01) != (atk31F4 & 0x01)) ||
          ((std::popcount(f5_1_bits) & 0x01) != ((atk31F5 >> 1) & 0x01)) ||
          ((std::popcount(f5_0_bits) & 0x01) != (atk31F5 & 0x01))) {
        continue;
      }
    }

    if (cur.depth == 0) {
      calc_next(thread_data.threadCheckDigits[0],
                thread_data.threadCheckDigits[1], 0x00);
    } else {
      calc_next(thread_data.threadCheckDigits[cur.depth],
                thread_data.threadCheckDigits[cur.depth + 1],
                cur.PW[cur.depth - 1]);
    }
    if (cur.depth == atk_count - 1) {
      // 最終文字はxorで決まる
      unsigned char c = atk31F9 ^ thread_data.threadCheckDigits[atk_count - 1]
                                      .F9[cur.PW[atk_count - 2]];
      if (c <= 0x35 && atoy[c] != '*') {
        cur.PW[atk_count - 1] = c;
        BulkCheckDigits &cd = thread_data.threadCheckDigits[atk_count];
        if (cd.F4[c] == atk31F4 && cd.F5[c] == atk31F5 && cd.F7[c] == atk31F7 &&
            cd.F8[c] == atk31F8 && cd.F9[c] == atk31F9 && cd.FA[c] == atk31FA &&
            cd.FB[c] == atk31FB) {
          // 標準出力を同時出力しないようにロック
          std::lock_guard<std::mutex> lock(mtx);
          printf("Hit: ");
          for (int idx = 0; idx < atk_count; idx++) {
            printf("%02X ", cur.PW[idx]);
          }
          printf(": ");
          for (int idx = 0; idx < atk_count; idx++) {
            printf("%c", atoy[cur.PW[idx]]);
          }
          printf("\n");
          // 発見したら抜ける
          // break;
        }
      }
    } else {
      for (int i = 0; i < 42; i++) {
        unsigned char c = itoa[i];
        DfsNode n;
        n.depth = cur.depth + 1;
        memcpy(n.PW, cur.PW, cur.depth + 1);
        n.PW[cur.depth] = c;
        pool.push_back(std::move(n));
      }
    }
  }
  {
    std::lock_guard<std::mutex> lock(mtx);
    printf("End id:%d %c,%c Count:%llu\n", thread_data.threadId,
           atoy[thread_data.PW1], atoy[thread_data.PW2], count);
  }
  thread_data.isFinished = true;
}

// スレッドが使うデータ
ThreadData thread_data[42 * 42];

// メイン
int main(int argc, char *argv[]) {
  printf("yokai-test03 brute force atk\n");

  int i = 0, j = 0;

  int atk_count = 1;
  int atk31F4 = 0, atk31F5 = 0, atk31F7 = 0, atk31F8 = 0, atk31F9 = 0,
      atk31FA = 0, atk31FB = 0;
  unsigned char start_a31DC[16];
  memset(start_a31DC, 0, sizeof(start_a31DC));

  int continue_count = 0;

  // アタック目標値を設定
  if (argc < 9) {
    printf("yokai03 - brute force atk 20211219\n");
    printf("usage:yokai03.exe $31F4 $31F5 $31F6 $31F7 $31F8 $31F9 $31FA $31FB "
           "{continue param}\n");
    printf("(例) yokai03.exe 00 08 03 22 88 20 .. パスワード<HAL>を検出\n");
    printf("continue paramを指定すると続きから再開できます。\n");
    printf("yokai03.exe \n");
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

  printf("解析パスワード文字数 : %d 文字\n", atk_count);

  // コンテニュー
  if (argc > 9) {
    continue_count = argc - 9;
    printf("プレフィクス指定で動作します prefix: ");
    for (i = 0; i < continue_count; i++) {
      start_a31DC[i] = (unsigned char)strtoul(argv[9 + i], NULL, 16);
      printf("%02X ", start_a31DC[i]);
    }
    printf("\n");
  } else {
    printf("最初から回します\n");
  }

  BulkCheckDigits checkDigits[PWLEN_MAX];
  for (int idx = 0; idx < BULK_SIZE; idx++) {
    checkDigits[0].F4[idx] = 0;
    checkDigits[0].F5[idx] = 0;
    checkDigits[0].F7[idx] = 0;
    checkDigits[0].F8[idx] = 0;
    checkDigits[0].F9[idx] = 0;
    checkDigits[0].FA[idx] = 0;
    checkDigits[0].FB[idx] = 0;
  }

  DfsNode initNode;
  initNode.depth = 0;
  for (int idx = 0; idx < PWLEN_MAX; idx++)
    initNode.PW[idx] = 0x00;

  if (continue_count > 0) {
    calc_next(checkDigits[0], checkDigits[1], 0x00);
    for (i = 1; i <= continue_count; i++) {
      calc_next(checkDigits[i], checkDigits[i + 1], start_a31DC[i - 1]);
      initNode.PW[i - 1] = start_a31DC[i - 1];
    }
    initNode.depth = continue_count;
  }

  // スレッドにばらまくためのデータ準備
  printf("スレッドにばらまくためのデータ準備中\n");
  if (initNode.depth == 0) {
    calc_next(checkDigits[initNode.depth], checkDigits[initNode.depth + 1],
              0x00);
  } else {
    calc_next(checkDigits[initNode.depth], checkDigits[initNode.depth + 1],
              initNode.PW[initNode.depth - 1]);
  }
  for (i = 0; i < 42; i++) {
    unsigned char pw1 = itoa[i];
    calc_next(checkDigits[initNode.depth + 1], checkDigits[initNode.depth + 2],
              pw1);
    for (j = 0; j < 42; j++) {
      int id = i * 42 + j;
      unsigned char pw2 = itoa[j];
      ThreadData &td = thread_data[id];
      td.threadId = id;
      td.isFinished = false;
      td.PW1 = pw1;
      td.PW2 = pw2;
      td.startNode.depth = initNode.depth + 2;
      memcpy(td.startNode.PW, initNode.PW, PWLEN_MAX);
      td.startNode.PW[initNode.depth] = pw1;
      td.startNode.PW[initNode.depth + 1] = pw2;
      for (int idx = 0; idx < PWLEN_MAX; idx++) {
        memcpy(td.threadCheckDigits[idx].F4, checkDigits[idx].F4, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].F5, checkDigits[idx].F5, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].F7, checkDigits[idx].F7, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].F8, checkDigits[idx].F8, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].F9, checkDigits[idx].F9, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].FA, checkDigits[idx].FA, BULK_SIZE);
        memcpy(td.threadCheckDigits[idx].FB, checkDigits[idx].FB, BULK_SIZE);
      }
    }
  }
  printf("準備完了\n");

  int create_count = 0;

  int cpu_count = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  std::vector<int> current_targets;
  std::mutex mtx;

  // 最初はCPUコア数分作る
  for (i = 0; i < cpu_count; i++) {
    threads.push_back(std::thread([&, create_count]() {
      calc_thread(thread_data[create_count], mtx, atk_count, atk31F4, atk31F5,
                  atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);
    }));
    current_targets.push_back(create_count);
    create_count++;
  }

  // 100us間隔でポーリングして終わり次第、次を始めさせる
  while (create_count < 42 * 42) {
    for (i = 0; i < cpu_count; i++) {
      if (thread_data[current_targets[i]].isFinished) {
        threads[i].join();
        threads[i] = std::thread([&, create_count]() {
          calc_thread(thread_data[create_count], mtx, atk_count, atk31F4,
                      atk31F5, atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);
        });
        current_targets[i] = create_count;
        create_count++;
        if (create_count >= 42 * 42) {
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
  for (i = 0; i < cpu_count; i++) {
    threads[i].join();
  }
  return 0;
}
