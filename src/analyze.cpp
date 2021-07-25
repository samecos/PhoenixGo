#include "analyze.h"

void MCTSEngine::OutputAnalysis(TreeNode* parent) {
    // We need to make a copy of the data before sorting
    auto sortable_data = std::vector<OutputAnalysisData>();

    if (parent->ch_len == 0) { // nothing to print
        return;
    }

    for (int i = 0; i < parent->ch_len; ++i) {
        TreeNode* node = parent->ch;

        std::string move = GoFunction::IdToMoveStr(node[i].move);

        // TODO: use a better way to get pv
        std::string pv = move + " " + m_debugger.GetMainMovePaths(i);

        // Not sure the meaning of value
        float root_action = (float)node[i].total_action / k_action_value_base / node[i].visit_count;
        float move_eval = (root_action + 1) * 50 * 100;
        float policy = node[i].prior_prob * 100;
        sortable_data.emplace_back(move, node[i].visit_count, move_eval, policy, pv);
    }
    std::stable_sort(std::begin(sortable_data), std::end(sortable_data));

    // Output analysis data in gtp stream
    auto i = 0;
    for (const auto& node : sortable_data) {
        if (i > 0) {
            std::cerr << " ";
        }
        std::cerr << node.get_info_string(i);
        i++;
    }
    std::cerr << "\n";
}