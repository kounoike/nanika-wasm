// yokai03mc.cpp : このファイルには 'main'
// 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <bit>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <time.h>
#include <vector>

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

void calc_next(const BulkCheckDigits &prevCD, BulkCheckDigits &nextCD,
               unsigned char prevChar) {
  unsigned char PW[BULK_SIZE], A[BULK_SIZE], A1[BULK_SIZE], C[BULK_SIZE],
      C1[BULK_SIZE];
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
  // for (int idx = 0; idx < BULK_SIZE; idx++)
  //   A[idx] = std::popcount(PW[idx]);
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

// メイン
int main(int argc, char *argv[]) {

  printf("yokai-test03 brute force atk\n");

  int i = 0, j = 0;
  int ret;
  // int stackApos = 0, stackXpos = 0, stackYpos = 0;
  // static int stackA[256];

  int atk_count = 1;
  int atk31F4 = 0, atk31F5 = 0, atk31F7 = 0, atk31F8 = 0, atk31F9 = 0,
      atk31FA = 0, atk31FB = 0;
  unsigned char start_a31DC[16];
  memset(start_a31DC, 0, sizeof(start_a31DC));

  int continue_count = 0;

  time_t timer;
  struct tm *local_time;

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

  // memset(stackA, 0, sizeof(stackA));

  timer = time(NULL);
  local_time = localtime(&timer);
  printf("%02d:%02d:%02d - ", local_time->tm_hour, local_time->tm_min,
         local_time->tm_sec);
  // printf("解析開始(ESCキーでコンテニュー値を表示して終了)\n");

  // コンテニュー
  if (argc > 9) {
    continue_count = argc - 9;
    printf("前回の続きからコンテニューします : ");
    for (i = 0; i < continue_count; i++) {
      start_a31DC[i] = (unsigned char)strtoul(argv[9 + i], NULL, 16);
      printf("%02X ", start_a31DC[i]);
    }
    printf("\n");
  } else {
    printf("最初から回します\n");
  }

  BulkCheckDigits checkDigits[atk_count + 1];
  for (int idx = 0; idx < BULK_SIZE; idx++) {
    checkDigits[0].F4[idx] = 0;
    checkDigits[0].F5[idx] = 0;
    checkDigits[0].F7[idx] = 0;
    checkDigits[0].F8[idx] = 0;
    checkDigits[0].F9[idx] = 0;
    checkDigits[0].FA[idx] = 0;
    checkDigits[0].FB[idx] = 0;
  }

  std::vector<DfsNode> pool;

  DfsNode initNode;
  initNode.depth = 0;
  for (int idx = 0; idx < PWLEN_MAX; idx++)
    initNode.PW[idx] = 0x00;
  pool.push_back(std::move(initNode));

  int count = 0;
  while (!pool.empty()) {
    count++;
    DfsNode cur = pool.back();
    pool.pop_back();
    // printf("count: %8d, cur.depth: %d cur.PW: %02X %02X %02X.. : %s \n",
    // count,
    //        cur.depth, cur.PW[0], cur.PW[1], cur.PW[2],
    //        pw_to_string(cur.PW, cur.depth).c_str());
    if (cur.depth == 0) {
      calc_next(checkDigits[0], checkDigits[1], 0x00);
    } else {
      calc_next(checkDigits[cur.depth], checkDigits[cur.depth + 1],
                cur.PW[cur.depth - 1]);
    }
    if (cur.depth == atk_count - 1) {
      // 最終文字はxorで決まる
      unsigned char c =
          atk31F9 ^ checkDigits[atk_count - 1].F9[cur.PW[atk_count - 2]];
      cur.PW[atk_count - 1] = c;
      BulkCheckDigits &cd = checkDigits[atk_count];
      if (cd.F4[c] == atk31F4 && cd.F5[c] == atk31F5 && cd.F7[c] == atk31F7 &&
          cd.F8[c] == atk31F8 && cd.F9[c] == atk31F9 && cd.FA[c] == atk31FA &&
          cd.FB[c] == atk31FB) {
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
        break;
      }
    } else {
      DfsNode nodes[42];
      for (int i = 0; i < 42; i++) {
        unsigned char c = itoa[i];
        unsigned char fb = checkDigits[cur.depth + 1].FB[c];
        if (fb > atk31FB || fb + 5 * (atk_count - cur.depth) < atk31FB) {
          continue;
        }
        nodes[i].depth = cur.depth + 1;
        memcpy(nodes[i].PW, cur.PW, cur.depth + 1);
        nodes[i].PW[cur.depth] = c;
        pool.push_back(std::move(nodes[i]));
      }
    }
  }
  printf("End\n");

  return 0;
}
