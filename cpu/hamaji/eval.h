#ifndef EVAL_H_
#define EVAL_H_

#include "../../core/decision.h"

#include <string>
#include <vector>

#include "base.h"
#include "mutex.h"

class LF;
class LP;

class Eval {
public:
  Eval();
  ~Eval();

  void fillEvalParamVector(const LF& field, vector<double>* param_scores,
                           int* chain_cnt = NULL);
  double evalFromParamVector(const vector<double>& param_scores);
  double eval(LP* plan);

  static const string getEvalString(const LP& plan);

  void study();

private:
  struct Param;
  struct Teacher;

  int getParamIndex(int id, int i);
  double getConnectionScore(const LF& field, vector<double>* param_scores);
  void getHeightScore(const LF& field, vector<double>* param_scores);
  void getVertical3(const LF& field, vector<double>* param_scores);

  double calcParamError(vector<Teacher>& teachers);

  static void init();
  static Mutex g_mu;
  static vector<Param> g_params;
  static vector<int> g_param_index_offsets;

  vector<Param> params_;
  vector<int> param_index_offsets_;
};

#endif  // EVAL_H_