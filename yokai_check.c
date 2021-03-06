// yokai03mc.cpp : このファイルには 'main'
// 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 文字コード変換テーブル
static unsigned char atoy[] = {
    'A', 'H', 'O', 'V', '1', '6', '*', '*', 'B', 'I', 'P', 'W', '2', '7',
    '*', '*', 'C', 'J', 'Q', 'X', '3', '8', '*', '*', 'D', 'K', 'R', 'Y',
    '4', '9', '*', '*', 'E', 'L', 'S', 'Z', '5', '0', '*', '*', 'F', 'M',
    'T', '-', 'n', '!', '*', '*', 'G', 'N', 'U', '.', 'm', 'c'};

// $31FB高速スキップテーブル
static int a31FBskip[] = {0, 1, 1, 2, 1, 2, 0, 0, 1, 2, 2, 3, 2, 3, 0, 0, 1, 2,
                          2, 3, 2, 3, 0, 0, 2, 3, 3, 4, 3, 4, 0, 0, 1, 2, 2, 3,
                          2, 3, 0, 0, 2, 3, 3, 4, 3, 4, 0, 0, 2, 3, 3, 4, 3, 4};

// 次の妥当文字
static unsigned char next_char[] = {
    /* A->H */ 0x01,
    /* H->O */ 0x02,
    /* O->V */ 0x03,
    /* V->1 */ 0x04,
    /* 1->6 */ 0x05,
    /* 6->B */ 0x08,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* B->I */ 0x09,
    /* I->P */ 0x0A,
    /* P->W */ 0x0B,
    /* W->2 */ 0x0C,
    /* 2->7 */ 0x0D,
    /* 7->C */ 0x10,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* C->J */ 0x11,
    /* J->Q */ 0x12,
    /* Q->X */ 0x13,
    /* X->3 */ 0x14,
    /* 3->8 */ 0x15,
    /* 8->D */ 0x18,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* D->K */ 0x19,
    /* K->R */ 0x1a,
    /* R->Y */ 0x1b,
    /* Y->4 */ 0x1c,
    /* 4->9 */ 0x1d,
    /* 9->E */ 0x20,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* E->L */ 0x21,
    /* L->S */ 0x22,
    /* S->Z */ 0x23,
    /* Z->5 */ 0x24,
    /* 5->0 */ 0x25,
    /* 0->F */ 0x28,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* F->M */ 0x29,
    /* M->T */ 0x2a,
    /* T->- */ 0x2b,
    /* -->n */ 0x2c,
    /* n->! */ 0x2d,
    /* !->G */ 0x30,
    /* *->* */ 0xff,
    /* *->* */ 0xff,
    /* G->N */ 0x31,
    /* N->U */ 0x32,
    /* U->. */ 0x33,
    /* .->m */ 0x34,
    /* m->c */ 0x35,
    /* c->Next */ 0x36,
};

#define A31FBDIFF 2

// check digit計算
// 0: 見つからず
// 非0: 見つかった
int calc_digit(unsigned char *a31DC, int atk_count, int a31FBsum, int a31F9tmp,
               unsigned char atk31F4, unsigned char atk31F5,
               unsigned char atk31F7, unsigned char atk31F8,
               unsigned char atk31F9, unsigned char atk31FA,
               unsigned char atk31FB) {
  int a31F6 = 0; // 文字列長さ
  int a31F4 = 0, a31F5 = 0, a31F7 = 0, a31F8 = 0, a31F9 = 0, a31FA = 1,
      a31FB = 0;
  int A = 0, X = 0, Y = 0, C = 0, Z = 0;
  int i, j;
  unsigned char temp = 0, temp2 = 0;
  int ror = 0;

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

  a31FB = a31FBsum;
  a31F9 = a31F9tmp;

  for (X = 0; X < atk_count; X++) {
    A = a31DC[X];

    for (Y = 0; Y < 8; Y++) {
      A = A << 1;

      if (A > 0xFF) {
        C = 1;
        A = A & 0xFF;
      } else {
        C = 0;
      }
      temp = A;
      // 31F4と31F5を右1ビットローテート
      ror = a31F4 & 0x01;
      a31F4 = a31F4 >> 1;
      a31F4 = a31F4 | (C << 7); // C0000000
      C = ror;

      ror = a31F5 & 0x01;
      a31F5 = a31F5 >> 1;
      a31F5 = a31F5 | (C << 7); // C0000000
      C = ror;

      // printf("ror %02X %02X\n",a31F4,a31F5);

      A = 0;
      A = 0xFF + C;
      if (A > 0xFF) {
        A = 0;
        C = 1;
      } else
        C = 0;
      A = A ^ 0xFF;
      temp2 = A;
      A = A & 0x84;
      A = A ^ a31F4;
      a31F4 = A;
      A = temp2;
      A = A & 0x08;
      A = A ^ a31F5;
      a31F5 = A;
      A = temp;
    }
    // ここまでで31F4と31F5算出完了

    // D8A4: // 31F7と31F8を生成(Complete)
    A = a31F4;
    if (A >= 0xE5) {
      C = 1;
    } else
      C = 0; // C5の値でキャリーを生成
    A = a31DC[X];
    A = A + a31F7 + C;
    if (A > 0xFF) { // ADCのキャリー処理
      A = A & 0xFF;
      C = 1;
    } else
      C = 0;
    a31F7 = A;
    A = a31F8;
    A = A + a31F5 + C;
    if (A > 0xFF) { // ADCのキャリー処理
      A = A & 0xFF;
      C = 1;
    } else
      C = 0;
    a31F8 = A;
    A = a31DC[X];

    // 31F9生成をスキップ、計算済みの値を使う(kounoike)
    // // D89B: // 31F9を生成(Complete)
    // stackA[stackApos++] = A;
    // A = A ^ a31F9;
    // a31F9 = A;
    // A = stackA[--stackApos];

    // D88F: // 31FAを生成
    // 31FAをローテート
    ror = a31FA & 0x01;
    a31FA = a31FA >> 1;
    a31FA = a31FA | (C << 7); // $31F8のCがここで入る
    C = ror;
    A = A + a31FA + C;
    if (A > 0xFF) { // ADCのキャリー処理
      A = A & 0xFF;
      C = 1;
    } else
      C = 0;
    a31FA = A;

    // 31FB生成をスキップ、計算済みの値にキャリー値のみ加算(kounoike)
    if (C > 0) {
      printf("C:1 X:%d\n", X);
    }
    a31FB += C;

    //   // D87F:
    //   stackA[stackApos++] = A;
    // D880: // 31FBを生成
    //   // Aを左ローテート
    //   A = A << 1;
    //   if (A > 0xFF) { // ADCのキャリー処理
    //     A = A & 0xFF;
    //     C = 1;
    //   } // ここにelseが入っていないのはバグ？(kounoike)
    //   if (A == 0)
    //     Z = 1;
    //   else
    //     Z = 0; // 演算結果がゼロの時Z=1;

    //   stackA[stackApos++] = A; // スタックに値を保存
    //   A = a31FB;
    //   A = A + C;
    //   if (A > 0xFF) { // ADCのキャリー処理
    //     A = A & 0xFF;
    //     C = 1;
    //   } else
    //     C = 0;
    //   a31FB = A;

    //   A = stackA[--stackApos];
    // if (!Z)
    //   goto D880; // ローテ終わるまでループ
    // printf("a31FB=%x ",a31FB);

    // A = stackA[--stackApos];
  }

  // tmpを代入するようになったのでチェックプリント外す
  // if (a31F9tmp != a31F9) {
  //   printf("a31F9 not match! a31F9:[%02X] a31F9tmp:[%02X]\n", a31F9,
  //          a31F9tmp);
  // }

  // 検算終了後にチェック
  if (a31F4 == atk31F4 && a31F5 == atk31F5) {
    if (a31F7 == atk31F7 && a31F8 == atk31F8 && a31F9 == atk31F9 &&
        a31FA == atk31FA && a31FB == atk31FB) {
      timer = time(NULL);
      local_time = localtime(&timer);
      printf("%02d:%02d:%02d - ", local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec);
      printf("Hit! : ");
      for (i = 0; i < atk_count; i++) {
        printf("%02X ", a31DC[i]);
      }
      printf("= ");
      for (i = 0; i < atk_count; i++) {
        printf("%c", atoy[a31DC[i]]);
      }
      printf("\n");
      return 1;
    }
  }
  return 0;
}

// メイン
int main(int argc, char *argv[]) {

  printf("yokai-test03 brute force atk\n");
  unsigned char a31DC[256];
  int i = 0, j = 0;
  int ret;
  // int stackApos = 0, stackXpos = 0, stackYpos = 0;
  // static int stackA[256];

  int atk_count = 1;
  int atk31F4 = 0, atk31F5 = 0, atk31F7 = 0, atk31F8 = 0, atk31F9 = 0,
      atk31FA = 0, atk31FB = 0;

  int a31FBsum = 0;
  unsigned char a31F9tmp = 0;
  int ror = 0;
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

  // スタック配列クリア
  memset(a31DC, 0, sizeof(a31DC));
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
      a31DC[i] = (unsigned char)strtoul(argv[9 + i], NULL, 16);
      printf("%02X ", a31DC[i]);
    }
    printf("\n");
  } else {
    printf("最初から回します\n");
  }

  // a31DCにターゲット桁数の数値を入れて回転させて、値が一致するまでアタック

  // テストコード(あーすまん氏の$31FB合計値テーブル比較によるループスキップ)
  // 文字全部のテーブル値を足した結果との差が1以下で無ければgoto
  // LOOPさせて高速化
  // a31FB, a31F9の計算はdifferentialに行う
  a31FBsum = 0;
  for (i = 0; i < atk_count; i++) {
    a31FBsum += a31FBskip[a31DC[i]];
  }

  a31F9tmp = 0;
  for (i = 0; i < atk_count; i++) {
    a31F9tmp ^= a31DC[i];
  }

  int cnt = 0;

  cnt++;

  if (cnt % 1 == 0) {

    printf("Current: ");
    for (i = 0; i < atk_count; i++) {
      printf("%02X ", a31DC[i]);
    }
    printf("= ");
    for (i = 0; i < atk_count; i++) {
      printf("%c", atoy[a31DC[i]]);
    }
    printf("\n");
  }

  ret = calc_digit(a31DC, atk_count, a31FBsum, a31F9tmp, atk31F4, atk31F5,
                   atk31F7, atk31F8, atk31F9, atk31FA, atk31FB);

  return 0;
}
