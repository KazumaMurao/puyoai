#include "eval2.h"

#include <numeric>

#include "field.h"

Eval2::Eval2() {
}

Eval2::~Eval2() {
}

static double getConnectionScore(const LF& field) {
  static double CONNECTION_SCORES[] = {
    0.0, -0.01, 0.0, 0.01, 0.02, 0.02, 0.02,
  };

  double connection_score = 0;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 13; y++) {
      int num_colors[] = {
        0, 0, 0, 0, 0, 0, 0, 0
      };
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
          //printf("y=%d x=%d %d\n", y+dy, x+dx, field.f[y+dy][x+dx] + 1);
          num_colors[(int)field.Get(x+dx, y+dy)]++;
        }
      }
      for (int i = RED; i <= GREEN; i++) {
        // We need this check for 13 danme
        if (num_colors[i] > 4) {
          num_colors[i] = 4;
          //LOG(FATAL) << "x=" << x << " y=" << y << field.GetDebugOutput();
        }
        connection_score += CONNECTION_SCORES[num_colors[i]];
      }
    }
  }
  return connection_score;
}

double Eval2::eval(LP* plan) {
  const LF& f = plan->field;
  double score = 0.0;

  int ipc = 0, ucc = 0, vpc = 0;
  int cc = f.getBestChainCount(&ipc, &ucc, &vpc);
  score += cc;

  score += getConnectionScore(f);

  int heights[7];
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 14; y++) {
      heights[x] = y - 1;
      if (!f.Get(x, y))
        break;
    }
  }

  int h12 = heights[1] - heights[2];
  if (h12 < 0)
    score += h12 * 0.01;
  int h65 = heights[6] - heights[5];
  if (h65 < 0)
    score += h65 * 0.008;
  int h54 = heights[5] - heights[4];
  if (h54 < 0)
    score += h65 * 0.005;

  score -= heights[3] * 0.005;

  double ah12 = (heights[1] + heights[2]) * 0.5;
  if (ah12 > 9)
    score -= (ah12 - 9) * 0.004;
  double ah456 = (heights[4] + heights[5] + heights[6]) / 3.0;
  int ahdiff = ah12 - ah456;
  if (ahdiff > 3.0)
    score -= ahdiff * 0.01;
  if (ahdiff < -1.0)
    score += (ahdiff + 1.0) * 0.005;

  return score;
}
