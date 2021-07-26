#include "analyze.h"


OutputAnalysisData::OutputAnalysisData(std::string move, const int visits, const float winrate,
    const float policy_prior, std::string pv)
    : m_move(std::move(move)),
    m_visits(visits),
    m_winrate(winrate),
    m_policy_prior(policy_prior),
    m_pv(std::move(pv)) {}

std::string OutputAnalysisData::get_info_string(const int order) const {
    auto tmp = "info move " + m_move
        + " visits " + std::to_string(m_visits)
        + " winrate "
        + std::to_string(static_cast<int>(m_winrate))
        + " prior "
        + std::to_string(static_cast<int>(m_policy_prior))
        + " lcb "
        + "0";
    if (order >= 0) {
        tmp += " order " + std::to_string(order);
    }
    tmp += " pv " + m_pv;
    return tmp;
}

