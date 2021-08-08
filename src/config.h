#pragma once
#include <gflags/gflags.h>
#include <vector>
#include <thread>
#include <functional>
#include "task_queue.h"
#include "wait_group.h"
#include "mcts_config.h"
#include "go_comm.h"
#include "go_state.h"

DECLARE_bool(lizzie);

typedef std::function<void(int, std::vector<float>, float)> EvalCallback;
struct EvalTask
{
    std::vector<bool> features;
    EvalCallback callback;
};

struct EvalData
{
	float value = 0.0f;
	//std::vector<float> prob((GoComm::GOBOARD_SIZE + 1), 0.0f);
	std::vector<float> prob = std::vector<float>((GoComm::GOBOARD_SIZE + 1), 0.0f);  // Doesn't include kPass.

	// Constructor
	EvalData(){}
	EvalData(std::vector<float> p, float v) :prob(p), value(v) {}
	EvalData(const EvalData& d) : value(d.value), prob(d.prob) {}

	EvalData& operator=(const EvalData& d)
	{
		value = d.value;
		prob = d.prob;
		return *this;
	}
};

class EvalCache
{
public:

	std::mutex m_x;
	size_t m_max_eval_cache_size = 2000000;
	std::unordered_map<uint64_t, EvalData> m_prob_value_map;
	std::deque<uint64_t> m_order;

	// Constructor
	EvalCache()
	{

	};
	~EvalCache() {};

	void Resize(int size)
	{
		m_max_eval_cache_size = size;
		while (m_order.size() > m_max_eval_cache_size)
		{
			m_prob_value_map.erase(m_order.front());
			m_order.pop_front();
		}
	}

	void Init()
	{
		m_prob_value_map.clear();
		m_order.clear();
	}

	bool Probe(const GoState& b, EvalData& d, bool check_sym = true)
	{
		return Probe(b.GetZobristHashValue(), d, check_sym);
	}

	bool Probe(uint64_t key, EvalData& d, bool check_sym = true)
	{
		std::lock_guard<std::mutex> lk(m_x);
		auto itr = m_prob_value_map.find(key);
		if (itr == m_prob_value_map.end()) return false;
		d = itr->second;
		return true;
	}

	void Insert(uint64_t key, const EvalData d)
	{
		std::lock_guard<std::mutex> lk(m_x);
		if (m_prob_value_map.find(key) == m_prob_value_map.end()) AddCache(key, d);
	}

	void AddCache(uint64_t key, const EvalData d)
	{
		m_prob_value_map.emplace(key, d);
		m_order.push_back(key);
		if (m_order.size() > m_max_eval_cache_size) {
			m_prob_value_map.erase(m_order.front());
			m_order.pop_front();
		}
	}
};

extern TaskQueue<EvalTask> g_eval_task_queue;
extern std::vector<std::thread> g_eval_with_batch_threads;
extern WaitGroup g_eval_threads_init_wg;
extern WaitGroup g_eval_tasks_wg;

extern std::unique_ptr<MCTSConfig> g_config;
extern std::thread g_analyze_thread;
extern EvalCache* g_eval_cache;
