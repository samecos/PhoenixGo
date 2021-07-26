#pragma once

#include <string>
#include "mcts_engine.h"
#include "config.h"

class OutputAnalysisData {
public:
    OutputAnalysisData(std::string move, const int visits, const float winrate,
        const float policy_prior, std::string pv);

    std::string get_info_string(const int order) const;
    
    friend bool operator<(const OutputAnalysisData& a,
        const OutputAnalysisData& b) {
        if (a.m_visits == b.m_visits) {
            return a.m_winrate < b.m_winrate;
        }
        return a.m_visits < b.m_visits;
    }
private:
    std::string m_move;
    int m_visits;
    float m_winrate;
    float m_policy_prior;
    std::string m_pv;

};



