  auto cmp = [](Process left, Process right)
  { if(left.pid!=right.pid){return (left.period) >= (right.period);}
    else{return left.remaining_time>=right.remaining_time;} };
  priority_queue<Process, vector<Process>, decltype(cmp)> Queue(cmp);