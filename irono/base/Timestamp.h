#pragma once

#include "copyable.h"


#include <boost/operators.hpp>

namespace irono {

//Timestamp 类成员只有唯一一个，记录从UNIX纪元初至现在的时间.
//推荐pass by value，底层直接使用寄存器
class Timestamp : public irono::copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp> {
public:

    Timestamp() : microSecondsSinceEpoch_(0) {}
    explicit Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_;}
    std::string toString() const;
    std::string toFormattedString() const;
    static Timestamp now();
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
private:
    int64_t microSecondsSinceEpoch_;
};
/// 实现一个自动实现其余三个，因为继承了less_than_comparable
inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
/// 实现一个自动实现!=，因为继承了equality_comparable
inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
//获得seconds后的时间辍
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

} // namespace irono