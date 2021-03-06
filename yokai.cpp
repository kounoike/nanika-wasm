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

// 次の妥当文字
static unsigned char next_char[] = {
    /* A->H */ 0x01,
    /* H->O */ 0x02,
    /* O->V */ 0x03,
    /* V->1 */ 0x04,
    /* 1->6 */ 0x05,
    /* 6->B */ 0x08,
    /* *->B */ 0x08,
    /* *->B */ 0x08,
    /* B->I */ 0x09,
    /* I->P */ 0x0A,
    /* P->W */ 0x0B,
    /* W->2 */ 0x0C,
    /* 2->7 */ 0x0D,
    /* 7->C */ 0x10,
    /* *->C */ 0x10,
    /* *->C */ 0x10,
    /* C->J */ 0x11,
    /* J->Q */ 0x12,
    /* Q->X */ 0x13,
    /* X->3 */ 0x14,
    /* 3->8 */ 0x15,
    /* 8->D */ 0x18,
    /* *->D */ 0x18,
    /* *->D */ 0x18,
    /* D->K */ 0x19,
    /* K->R */ 0x1a,
    /* R->Y */ 0x1b,
    /* Y->4 */ 0x1c,
    /* 4->9 */ 0x1d,
    /* 9->E */ 0x20,
    /* *->E */ 0x20,
    /* *->E */ 0x20,
    /* E->L */ 0x21,
    /* L->S */ 0x22,
    /* S->Z */ 0x23,
    /* Z->5 */ 0x24,
    /* 5->0 */ 0x25,
    /* 0->F */ 0x28,
    /* *->F */ 0x28,
    /* *->F */ 0x28,
    /* F->M */ 0x29,
    /* M->T */ 0x2a,
    /* T->- */ 0x2b,
    /* -->n */ 0x2c,
    /* n->! */ 0x2d,
    /* !->G */ 0x30,
    /* *->G */ 0x30,
    /* *->G */ 0x30,
    /* G->N */ 0x31,
    /* N->U */ 0x32,
    /* U->. */ 0x33,
    /* .->m */ 0x34,
    /* m->c */ 0x35,
    /* c->Next */ 0x36,
};

// check digit計算
// 64個まとめて計算することでSIMD命令に置き換えてくれてることを期待している
// 0: 見つからず 非0: 見つかった
int bulk_calc_digit(unsigned char bulk_a31DC[16][BULK_SIZE], int atk_count,
                    unsigned char *bulk_a31FBsum, unsigned char atk31F4,
                    unsigned char atk31F5, unsigned char atk31F7,
                    unsigned char atk31F8, unsigned char atk31FA,
                    unsigned char atk31FB) {
  alignas(64) unsigned char a31F4[BULK_SIZE], a31F5[BULK_SIZE],
      a31F7[BULK_SIZE], a31F8[BULK_SIZE], a31F9[BULK_SIZE], a31FA[BULK_SIZE],
      a31FB[BULK_SIZE];
  alignas(64) unsigned char A[BULK_SIZE], C[BULK_SIZE];
  alignas(64) unsigned char C1[BULK_SIZE];
  int i, j, X, Y;

  memset(a31F4, 0, sizeof(a31F4));
  memset(a31F5, 0, sizeof(a31F5));
  memset(a31F7, 0, sizeof(a31F7));
  memset(a31F8, 0, sizeof(a31F8));
  memset(a31FA, 0, sizeof(a31FA));
  memcpy(a31FB, bulk_a31FBsum, sizeof(a31FB));
  memset(A, 0, sizeof(A));
  memset(C, 0, sizeof(C));
  memset(C1, 0, sizeof(C1));

  time_t timer;
  struct tm *local_time;

  // 以下メインルーチン
  // printf("Skip logic passed: ");
  // for (i = 0; i < atk_count; i++) {
  //   printf("%02X ", a31DC[i]);
  // }
  // printf("= ");
  // for (i = 0; i < atk_count; i++) {
  //   printf("%c", atoy[a31DC[i]]);
  // }
  // printf("\n");

  for (X = 0; X < atk_count; X++) {
    for (int idx = 0; idx < BULK_SIZE; idx++)
      A[idx] = bulk_a31DC[X][idx];

    for (Y = 0; Y < 8; Y++) {
      for (int idx = 0; idx < BULK_SIZE; idx++)
        C[idx] = A[idx] & 0x80; // どうせ7bitシフトして戻す
      for (int idx = 0; idx < BULK_SIZE; idx++)
        A[idx] <<= 1;

      // 31F4と31F5を右1ビットローテート
      for (int idx = 0; idx < BULK_SIZE; idx++)
        C1[idx] = a31F4[idx] & 0x01;
      for (int idx = 0; idx < BULK_SIZE; idx++)
        a31F4[idx] = (a31F4[idx] >> 1) | C[idx];

      for (int idx = 0; idx < BULK_SIZE; idx++)
        C[idx] = a31F5[idx] & 0x01;
      for (int idx = 0; idx < BULK_SIZE; idx++)
        a31F5[idx] = (a31F5[idx] >> 1) | (C1[idx] << 7);

      for (int idx = 0; idx < BULK_SIZE; idx++)
        a31F4[idx] ^= C[idx] > 0 ? 0x84 : 0;
      for (int idx = 0; idx < BULK_SIZE; idx++)
        a31F5[idx] ^= C[idx] > 0 ? 0x08 : 0;
    }
    // ここまでで31F4と31F5算出完了

    // D8A4: // 31F7と31F8を生成(Complete)
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = a31F4[idx] >= 0xE5 ? 1 : 0;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31F7[idx] = bulk_a31DC[X][idx] + a31F7[idx] + C[idx];
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] = (a31F7[idx] < bulk_a31DC[X][idx] ||
                (C[idx] > 0 && a31F7[idx] == bulk_a31DC[X][idx]))
                   ? 1
                   : 0; // ADCのキャリー処理
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31F8[idx] = a31F8[idx] + a31F5[idx] + C[idx];
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C[idx] =
          (a31F8[idx] < a31F5[idx] || (C[idx] > 0 && a31F8[idx] == a31F5[idx]))
              ? 1
              : 0; // ADCのキャリー処理

    // 31F9生成をスキップ、計算済みの値を使う(kounoike)
    // // D89B: // 31F9を生成(Complete)
    // stackA[stackApos++] = A;
    // A = A ^ a31F9;
    // a31F9 = A;
    // A = stackA[--stackApos];

    // D88F: // 31FAを生成
    // 31FAをローテート
    for (int idx = 0; idx < BULK_SIZE; idx++)
      C1[idx] = a31FA[idx] & 0x01;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31FA[idx] = a31FA[idx] >> 1;
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31FA[idx] = a31FA[idx] | (C[idx] << 7); // $31F8のCがここで入る
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31FA[idx] = bulk_a31DC[X][idx] + a31FA[idx] + C1[idx];
    // 31FB生成をスキップ、計算済みの値にキャリー値のみ加算(kounoike)
    for (int idx = 0; idx < BULK_SIZE; idx++)
      a31FB[idx] += (a31FA[idx] < bulk_a31DC[X][idx] ||
                     (C1[idx] > 0 && a31FA[idx] == bulk_a31DC[X][idx]))
                        ? 1
                        : 0; // ADCのキャリー処理
  }

  // 検算終了後にチェック
  for (int idx = 0; idx < BULK_SIZE; idx++) {
    if (a31F4[idx] == atk31F4 && a31F5[idx] == atk31F5) {
      if (a31F7[idx] == atk31F7 && a31F8[idx] == atk31F8 &&
          a31FA[idx] == atk31FA && a31FB[idx] == atk31FB) {
        char filename[300];
        char ext[] = ".hit.txt";
        for (i = 0; i < atk_count; i++) {
          filename[i] = atoy[bulk_a31DC[i][idx]];
        }
        strncpy(&filename[atk_count], ext, strlen(ext) + 1);
        FILE *fp = fopen(filename, "w");
        timer = time(NULL);
        local_time = localtime(&timer);
        printf("%02d:%02d:%02d - ", local_time->tm_hour, local_time->tm_min,
               local_time->tm_sec);
        printf("Hit! : ");
        fprintf(fp, "Hit! : ");
        for (i = 0; i < atk_count; i++) {
          printf("%02X ", bulk_a31DC[i][idx]);
          fprintf(fp, "%02X ", bulk_a31DC[i][idx]);
        }
        printf("= ");
        fprintf(fp, "= ");
        for (i = 0; i < atk_count; i++) {
          printf("%c", atoy[bulk_a31DC[i][idx]]);
          fprintf(fp, "%c", atoy[bulk_a31DC[i][idx]]);
        }
        printf("\n");
        fprintf(fp, "\n");
        fclose(fp);
        return 1;
      }
    }
  }
  return 0;
}

bool found = false;

void calc_thread(int thread_id, const unsigned char *start_a31DC, int atk_count,
                 unsigned char atk31F4, unsigned char atk31F5,
                 unsigned char atk31F7, unsigned char atk31F8,
                 unsigned char atk31F9, unsigned char atk31FA,
                 unsigned char atk31FB) {
  int bulk_idx = 0;
  unsigned char a31DC[16];
  unsigned char bulk_a31DC[16][BULK_SIZE];
  unsigned char a31FBsum = 0, a31F9tmp = 0;
  unsigned char bulk_a31FBsum[BULK_SIZE];
  int i, j, ret;

  // 最後から2文字目に使う文字
  unsigned char thread_char = itoa[thread_id];

  // スタック配列クリア
  memcpy(a31DC, start_a31DC, sizeof(a31DC));

  a31F9tmp = 0;
  for (i = 0; i < atk_count - 2; i++) {
    a31F9tmp ^= a31DC[i];
  }
  a31DC[atk_count - 2] = thread_char;
  a31F9tmp ^= thread_char;
  a31DC[atk_count - 1] = atk31F9 ^ a31F9tmp;

  a31FBsum = 0;
  for (i = 0; i < atk_count; i++) {
    a31FBsum += std::popcount(a31DC[i]);
  }

  int cnt = 0;

  printf("thread id:[%d] char code: [%02X] char: [%c]\n", thread_id,
         thread_char, atoy[thread_char]);

  while (!found) {
    int need_check = 1;
    cnt++;

    // スタート

    // // 試しにこのタイミングで配列を全走査して
    // // atoy[]に'*'を検出したら強制スキップさせて 高速化できないか実験
    // // 2桁目以降に出現した場合は上位インクリメントして下位をゼロクリア
    // for (i = 0; i < atk_count; i++) {
    //   if (atoy[a31DC[i]] == '*') {
    //     need_check = 0;
    //     break;
    //   }
    // }
    if (a31FBsum > atk31FB || a31FBsum + atk_count < atk31FB) {
      need_check = 0;
    }

    if (thread_id == 0 && cnt % 10000000 == 0) {

      printf("Current: ");
      for (i = 0; i < atk_count; i++) {
        printf("%02X ", a31DC[i]);
      }
      printf("= ");
      for (i = 0; i < atk_count; i++) {
        printf("%c", atoy[a31DC[i]]);
      }
      printf(" : need_check:[%d] a31FBsum:[%02X] a31F9tmp:[%02X]\n", need_check,
             a31FBsum, a31F9tmp);
    }

    if (need_check) {
      for (i = 0; i < atk_count; i++) {
        bulk_a31DC[i][bulk_idx] = a31DC[i];
      }
      bulk_a31FBsum[bulk_idx] = a31FBsum;
      bulk_idx++;
      if (bulk_idx == BULK_SIZE) {
        ret = bulk_calc_digit(bulk_a31DC, atk_count, bulk_a31FBsum, atk31F4,
                              atk31F5, atk31F7, atk31F8, atk31FA, atk31FB);
        if (ret) {
          // 見つかった
          found = true;
          // return;
        }
        bulk_idx = 0;
      }
    }

    do {
      // 最後の1文字はxorで決める。
      // 最後から2文字目はスレッドごとに決めた文字
      int loop_start = atk_count - 3;
      a31FBsum -= std::popcount(a31DC[atk_count - 1]);
      a31FBsum -= std::popcount(a31DC[atk_count - 2]);
      a31F9tmp ^= a31DC[atk_count - 2];

      // a31FBsumが足りないときは大きく動かす
      if (a31FBsum + atk_count + std::popcount(thread_char) + 4 < atk31FB) {
        int partial_sum = a31FBsum;
        for (i = loop_start; i >= 0; i--) {
          // 後の桁から4になる文字に置き換えて考えて、
          partial_sum += 4 - std::popcount(a31DC[i]);
          loop_start = i;
          // 超えたところで終了
          if (partial_sum + atk_count + std::popcount(thread_char) + 4 >=
              atk31FB) {
            break;
          }
        }
      }

      // 大きすぎるときも大きく動かす
      if (a31FBsum + std::popcount(thread_char) > atk31FB) {
        int partial_sum = a31FBsum;
        for (i = loop_start; i >= 0; i--) {
          partial_sum -= std::popcount(a31DC[i]);
          if (partial_sum + std::popcount(thread_char) <= atk31FB) {
            loop_start = i;
            break;
          }
        }
      }
      // printf("loop_start: [%d]\n", loop_start);

      for (i = loop_start + 1; i < atk_count - 2; i++) {
        a31FBsum -= std::popcount(a31DC[i]);
        a31F9tmp ^= a31DC[i];
        a31DC[i] = 0x00;
      }

      // 0x00-0x35の範囲でループさせる
      for (i = loop_start; i >= 0; i--) {
        // 1つずつ進める
        a31FBsum -= std::popcount(a31DC[i]);
        a31F9tmp ^= a31DC[i];
        // '*'にしない
        // do {
        //   a31DC[i]++;
        // } while (atoy[a31DC[i]] == '*');
        a31DC[i] = next_char[a31DC[i]];
        // 0x35を超えたら次の桁へ
        if (a31DC[i] > 0x35) {
          a31DC[i] = 0;
          // a31FBsum += a31FBskip[a31DC[i]];
          // a31F9tmp ^= a31DC[i];
        } else {
          a31FBsum += std::popcount(a31DC[i]);
          a31F9tmp ^= a31DC[i];
          break;
        }
        // 最初の文字が0x36になった瞬間に脱出
        if (i == 0) {
          // 抜ける前に残りを検索
          ret = bulk_calc_digit(bulk_a31DC, atk_count, bulk_a31FBsum, atk31F4,
                                atk31F5, atk31F7, atk31F8, atk31FA, atk31FB);
          if (ret) {
            // 見つかった
            found = true;
            return;
          }
          printf("Thread [%d]: Not Found.\n", thread_id);
          return;
        }
      }
      a31DC[atk_count - 2] = thread_char;
      a31F9tmp ^= a31DC[atk_count - 2];
      a31FBsum += std::popcount(a31DC[atk_count - 2]);
      a31DC[atk_count - 1] = atk31F9 ^ a31F9tmp;
      a31FBsum += std::popcount(a31DC[atk_count - 1]);
    } while (a31FBsum + atk_count < atk31FB || a31FBsum > atk31FB ||
             atoy[a31DC[atk_count - 1]] == '*');
  }
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

  std::vector<std::thread> threads;

  for (i = 0; i < 42; i++) {
    threads.push_back(std::thread([=]() {
      calc_thread(i, start_a31DC, atk_count, atk31F4, atk31F5, atk31F7, atk31F8,
                  atk31F9, atk31FA, atk31FB);
    }));
  }

  for (i = 0; i < 42; i++) {
    threads[i].join();
  }

  return 0;
}
