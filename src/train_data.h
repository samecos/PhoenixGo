#pragma once
#include <vector>
#include <glog/logging.h>
#include "config.h"
#include "go_comm.h"

class TrainData
{
public:
	TrainData():m_winner(GoComm::COLOR_UNKNOWN),m_policy(0.0f),m_feature(0)
	{}

	TrainData(std::vector<bool> feature, std::vector<float> policy, GoStoneColor winner)
	{
		CHECK_EQ(feature.size(), (GoComm::GOBOARD_SIZE * GoFeature::FEATURE_COUNT)) << "Output Train Feature Data: failed, "
			<< "feature data size is not equal to: " << (GoComm::GOBOARD_SIZE * GoFeature::FEATURE_COUNT)
			<< "  feature size is:  " << feature.size();
		CHECK_EQ(policy.size(), (GoComm::GOBOARD_SIZE + 1)) << "Output Policy TrainData: failed, "
			<< "policy data size is not equal to: " << (GoComm::GOBOARD_SIZE + 1)
			<< "  policy size is:  " << policy.size();
		m_feature.assign(feature.begin(), feature.end());
		m_policy.assign(policy.begin(), policy.end());
		m_winner = winner;
	}
	~TrainData()
	{
	}

public:
	GoStoneColor m_winner;
	std::vector<float> m_policy;
	std::vector<bool> m_feature;
};
