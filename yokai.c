// yokai03mc.cpp : このファイルには 'main'
// 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 文字コード変換テーブル
static char atoy[] = {'A', 'H', 'O', 'V', '1', '6', '*', '*', 'B', 'I', 'P',
                      'W', '2', '7', '*', '*', 'C', 'J', 'Q', 'X', '3', '8',
                      '*', '*', 'D', 'K', 'R', 'Y', '4', '9', '*', '*', 'E',
                      'L', 'S', 'Z', '5', '0', '*', '*', 'F', 'M', 'T', '-',
                      'n', '!', '*', '*', 'G', 'N', 'U', '.', 'm', 'c'};

// $31FB高速スキップテーブル
static int a31FBskip[] = {0, 1, 1, 2, 1, 2, 0, 0, 1, 2, 2, 3, 2, 3, 0, 0, 1, 2,
                          2, 3, 2, 3, 0, 0, 2, 3, 3, 4, 3, 4, 0, 0, 1, 2, 2, 3,
                          2, 3, 0, 0, 2, 3, 3, 4, 3, 4, 0, 0, 2, 3, 3, 4, 3, 4};

// メイン
int main(int argc, char *argv[]) {

  printf("yokai-test03 brute force atk\n");
  char a31DC[256];
  char temp = 0;
  int i = 0, j = 0;
  int stackApos = 0, stackXpos = 0, stackYpos = 0;
  static int stackA[256];

  int atk_count = 1;
  int atk31F4 = 0, atk31F5 = 0, atk31F7 = 0, atk31F8 = 0, atk31F9 = 0,
      atk31FA = 0, atk31FB = 0;

  int A = 0, X = 0, Y = 0, C = 0, Z = 0;
  int a31F6 = 0; // 文字列長さ
  int a31F4 = 0, a31F5 = 0, a31F7 = 0, a31F8 = 0, a31F9 = 0, a31FA = 0,
      a31FB = 0;
  int a31FBsum = 0;
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
  a31F6 = atk_count;
  atk31F7 = (int)strtoul(argv[4], NULL, 16);
  atk31F8 = (int)strtoul(argv[5], NULL, 16);
  atk31F9 = (int)strtoul(argv[6], NULL, 16);
  atk31FA = (int)strtoul(argv[7], NULL, 16);
  atk31FB = (int)strtoul(argv[8], NULL, 16);

  printf("解析パスワード文字数 : %d 文字\n", atk_count);

  // スタック配列クリア
  memset(a31DC, 0, sizeof(a31DC));
  memset(stackA, 0, sizeof(stackA));

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
  }

  // a31DCにターゲット桁数の数値を入れて回転させて、値が一致するまでアタック

  // テストコード(あーすまん氏の$31FB合計値テーブル比較によるループスキップ)
  // 文字全部のテーブル値を足した結果との差が1以下で無ければgoto
  // LOOPさせて高速化
  // 計算はdifferentialに行う
  a31FBsum = 0;
  for (i = 0; i < atk_count; i++) {
    a31FBsum += a31FBskip[a31DC[i]];
  }

  while (1) {
    int need_check = 1;

    // スタート
    X = 0;
    a31F4 = 0;
    a31F5 = 0;
    a31F7 = 0;
    a31F8 = 0;
    a31F9 = 0;
    a31FB = 0;
    A = 1;
    a31FA = A;
    stackApos = 0;
    C = 0;

    // // 試しにこのタイミングで配列を全走査して
    // // atoy[]に'*'を検出したら強制スキップさせて 高速化できないか実験
    // // 2桁目以降に出現した場合は上位インクリメントして下位をゼロクリア
    // for (i = 0; i < atk_count; i++) {
    //   if (atoy[a31DC[i]] == '*') {
    //     need_check = 0;
    //     break;
    //   }
    // }
    if (atk31FB != a31FBsum && atk31FB != a31FBsum + 1) {
      need_check = 0;
    }

    printf("Start: ");
    for (i = 0; i < atk_count; i++) {
      printf("%02X ", a31DC[i]);
    }
    printf("= ");
    for (i = 0; i < atk_count; i++) {
      printf("%c", atoy[a31DC[i]]);
    }
    printf(" : need_check:[%d] a31FBsum:[%d]\n", need_check, a31FBsum);

    if (need_check) {
      // 以下メインルーチン
      printf("Skip logic passed: ");
      for (i = 0; i < atk_count; i++) {
        printf("%02X ", a31DC[i]);
      }
      printf("= ");
      for (i = 0; i < atk_count; i++) {
        printf("%c", atoy[a31DC[i]]);
      }
      printf("\n");

    D86B:
      A = a31DC[X];

      // D8BD:
      stackA[stackApos++] = A;
      Y = 8;
    D8C0:
      A = A << 1;

      if (A > 0xFF) {
        C = 1;
        A = A & 0xFF;
      } else {
        C = 0;
      }
      stackA[stackApos++] = A;
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
      stackA[stackApos++] = A;
      A = A & 0x84;
      A = A ^ a31F4;
      a31F4 = A;
      A = stackA[--stackApos];
      A = A & 0x08;
      A = A ^ a31F5;
      a31F5 = A;
      A = stackA[--stackApos];
      Y--;
      if (Y > 0)
        goto D8C0;
      A = stackA[--stackApos]; // ここまでで31F4と31F5算出完了

      // D8A4: // 31F7と31F8を生成(Complete)
      stackA[stackApos++] = A;
      stackA[stackApos++] = A;
      A = a31F4;
      if (A >= 0xE5) {
        C = 1;
      } else
        C = 0; // C5の値でキャリーを生成
      A = stackA[--stackApos];
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
      A = stackA[--stackApos];

      // D89B: // 31F9を生成(Complete)
      stackA[stackApos++] = A;
      A = A ^ a31F9;
      a31F9 = A;
      A = stackA[--stackApos];

      // D88F: // 31FAを生成
      stackA[stackApos++] = A;
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

      A = stackA[--stackApos];

      // D87F:
      stackA[stackApos++] = A;
    D880: // 31FBを生成
      // Aを左ローテート
      A = A << 1;
      if (A > 0xFF) { // ADCのキャリー処理
        A = A & 0xFF;
        C = 1;
      }
      if (A == 0)
        Z = 1;
      else
        Z = 0; // 演算結果がゼロの時Z=1;

      stackA[stackApos++] = A; // スタックに値を保存
      A = a31FB;
      A = A + C;
      if (A > 0xFF) { // ADCのキャリー処理
        A = A & 0xFF;
        C = 1;
      } else
        C = 0;
      a31FB = A;

      A = stackA[--stackApos];
      if (!Z)
        goto D880; // ローテ終わるまでループ
      // printf("a31FB=%x ",a31FB);

      A = stackA[--stackApos];

      // 文字数分だけ演算をカウント
      X++;

      if (a31F6 != X)
        goto D86B;

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
        }
      }
    }

    // 0x00-0x35の範囲でループさせる
    // '*'にしない
    for (i = 0; i < atk_count; i++) {
      a31FBsum -= a31FBskip[a31DC[i]];
      do {
        a31DC[i]++; // 1個目をインクリメント
      } while (atoy[a31DC[0]] == '*');
      // 35を超えたら次の桁へ
      if (a31DC[i] > 0x35) {
        a31DC[i] = 0;
        a31FBsum += a31FBskip[a31DC[i]];
        a31FBsum -= a31FBskip[a31DC[i + 1]];
      } else {
        a31FBsum += a31FBskip[a31DC[i]];
        break;
      }
      // 最終桁が36になった瞬間に脱出
      if (a31DC[atk_count - 1] == 0x36) {
        printf("End.\n");
        return 0;
      }
    }

    // // ESCキー判定。65535回に1度しかチェックしない
    // if (a31DC[0] == 0 && a31DC[1] == 0 && a31DC[2] == 0 && a31DC[3] == 0 &&
    //     a31DC[4] == 0 && a31DC[5] == 0) {
    //   printf("continue command : yokai03.exe %s %s %s %s %s %s %s %s ",
    //   argv[1],
    //          argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]);
    //   for (i = 0; i < atk_count; i++) {
    //     printf("%02X ", a31DC[i]);
    //   }
    //   printf("\n");
    //   return 0;
    // }
  }

  return 0;
}
