#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <sys/wait.h>
#endif
#pragma once
#include <boost/asio.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <google/protobuf/util/message_differencer.h>

#include "str_utils.h"

#include "mcts_engine.h"
#include "eval_routine.h"
#include "analyze.h"


namespace asio = boost::asio;

DEFINE_string(config_path, "", "Path of mcts config file.");
DEFINE_string(gpu_list, "", "List of gpus used by neural network.");
DEFINE_string(init_moves, "", "Initialize Go board with init_moves.");
DEFINE_bool(gtp, false, "Run as gtp server.");
DEFINE_bool(selfplay, false, "Run as selfplay server");
DEFINE_int32(listen_port, 0, "Listen which port.");
DEFINE_int32(selfplay_thread_num, 0, "Nums of run async selfplay nums");
DEFINE_int32(games_num, 5000, "Total Games Num Run");
DEFINE_string(allow_ip, "", "List of client ip allowed to connect, seperated by comma.");
#if !defined(_WIN32) && !defined(_WIN64)
DEFINE_bool(fork_per_request, true, "Fork for each request or not.");
#endif
DEFINE_bool(lizzie, false, "output analyze data for Lizzie.");

const int k_handicaps_x[5] = {3, 15, 15, 3, 9};
const int k_handicaps_y[5] = {3, 15, 3, 15, 9};

std::unique_ptr<MCTSEngine> InitEngine(const std::string &config_path)
{
    InitConfig(config_path);
    CHECK(g_config != nullptr) << "Load mcts config file '" << config_path << "' failed";
    LOG(INFO) << "load config succ: \n" << g_config->DebugString();
    return std::unique_ptr<MCTSEngine>(new MCTSEngine(*g_config));
}

std::unique_ptr<MCTSEngine> InitSelfplayEngine()
{
    return std::unique_ptr<MCTSEngine>(new MCTSEngine(*g_config));
}

std::unique_ptr<MCTSEngine> InitSelfplayEngine(int id)
{
    return std::unique_ptr<MCTSEngine>(new MCTSEngine(*g_config, id));
}

void ReloadConfig(MCTSEngine &engine, const std::string &config_path)
{
    InitConfig(config_path);
    if (g_config == nullptr) {
        LOG(ERROR) << "Load mcts config file '" << config_path << "' failed";
        return;
    }
    if (google::protobuf::util::MessageDifferencer::Equals(engine.GetConfig(), *g_config)) {
        LOG(INFO) << "Config hasn't changed";
    } else {
        engine.SetPendingConfig(std::move(g_config));
    }
}

std::string EncodeMove(GoCoordId x, GoCoordId y)
{
    if (GoFunction::IsPass(x, y)) {
        return "pass";
    }
    if (GoFunction::IsResign(x, y)) {
        return "resign";
    }
    return std::string({x > 7 ? char('B' + x) : char('A' + x)}) + std::to_string(y + 1);
}

void DecodeMove(const std::string &s, GoCoordId &x, GoCoordId &y)
{
    if (s == "pass") {
        x = y = GoComm::COORD_PASS;
    } else if (s == "resign") {
        x = y = GoComm::COORD_RESIGN;
    } else {
        x = s[0] > 'i' ? s[0] - 'b' : s[0] - 'a';
        y = std::stoi(s.substr(1)) - 1;
    }
}

GoStoneColor DecodeColor(const std::string &s)
{
    if (s == "b" || s == "black") {
        return GoComm::BLACK;
    }
    if (s == "w" || s == "white") {
        return GoComm::WHITE;
    }
    return GoComm::COLOR_UNKNOWN;
}

std::pair<bool, std::string> GTPExecute(MCTSEngine &engine, const std::string &cmd)
{
    FLAGS_lizzie = false;
    std::string op;
    std::istringstream ss(cmd);
    ss >> op;
    if (op == "name") {
        return { true, "Leela Zero" };
    }
    if (op == "version") {
        return { true, "0.17" };
    }
    if (op == "protocol_version") {
        return {true, "2"};
    }
    if (op == "list_commands") {
        return {true, "name\nversion\nprotocol_version\nlist_commands\nquit\nclear_board\nboardsize\nkomi\ntime_settings\ntime_left\nplace_free_handicap\nset_free_handicap\nplay\ngenmove\nfinal_score\nget_debug_info\nget_last_move_debug_info\nundo\nlz-analyze"};
    }
    if (op == "quit") {
        return {true, ""};
    }
    if (op == "clear_board") {
        engine.Reset();
        return {true, ""};
    }
    if (op == "boardsize") {
        int size;
        ss >> size;
        if (size != 19) {
            return {false, "unacceptable size"};
        }
        engine.Reset();
        return {true, ""};
    }
    if (op == "komi") {
        float komi;
        ss >> komi;
        if (komi != 7.5) {
            return {false, "unacceptable komi"};
        }
        return {true, ""};
    }
    if (op == "time_settings") {
        int main_time, byo_yomi_time, byo_yomi_stones;
        ss >> main_time >> byo_yomi_time >> byo_yomi_stones;
        engine.GetByoYomiTimer().Set(main_time, byo_yomi_time);
        return {true, ""};
    }
    if (op == "time_left") {
        std::string color;
        int time, stones;
        ss >> color >> time >> stones;
        if (stones) {
            engine.GetByoYomiTimer().SetRemainTime(DecodeColor(color), 0);
        } else {
            engine.GetByoYomiTimer().SetRemainTime(DecodeColor(color), time);
        }
        return {true, ""};
    }
    if (op == "get_remain_time") {
        std::string color;
        ss >> color;
        return {true, std::to_string(engine.GetByoYomiTimer().GetRemainTime(DecodeColor(color)))};
    }
    if (op == "place_free_handicap") {
        int num_handicap = 0;
        ss >> num_handicap;
        if (num_handicap > 5) {
            return {false, "invalid number of stones"};
        }
        std::string output;
        for (int i = 0; i < num_handicap; i++) {
            if (i > 0) engine.Move(-1, -1);
            int x = k_handicaps_x[i];
            int y = k_handicaps_y[i];
            engine.Move(x, y);
            if (output.size()) output += " ";
            output = EncodeMove(x, y);
        }
        return {true, output};
    }
    if (op == "set_free_handicap") {
        std::string move;
        for (int i = 0; ss >> move; ++i) {
            if (i > 0) engine.Move(-1, -1);
            GoCoordId x, y;
            DecodeMove(move, x, y);
            engine.Move(x, y);
        }
        return {true, ""};
    }
    if (op == "play") {
        ReloadConfig(engine, FLAGS_config_path);
        std::string color, move;
        ss >> color >> move;
        GoCoordId x, y;
        DecodeMove(move, x, y);
        if (DecodeColor(color) != engine.GetBoard().CurrentPlayer()) {
            engine.Move(-1, -1);
        }
        engine.Move(x, y);
        std::cerr << engine.GetDebugger().GetLastMoveDebugStr() << std::endl;
        return {true, ""};
    }
    if (op == "genmove") {
        ReloadConfig(engine, FLAGS_config_path);
        std::string color;
        ss >> color;
        if (DecodeColor(color) != engine.GetBoard().CurrentPlayer()) {
            engine.Move(-1, -1);
        }
        GoCoordId x = -1, y = -1;
        engine.GenMove(x, y);
        engine.Move(x, y);
        std::cerr << engine.GetDebugger().GetLastMoveDebugStr() << std::endl;
        return {true, EncodeMove(x, y)};
    }
    if (op == "final_score") {
        GoStoneColor curr = engine.GetBoard().CurrentPlayer();
        return {true, curr == GoComm::BLACK ? "W+0.5" : "B+0.5"};
    }
    if (op == "get_debug_info") {
        return {true, engine.GetDebugger().GetDebugStr()};
    }
    if (op == "get_last_move_debug_info") {
        return {true, engine.GetDebugger().GetLastMoveDebugStr()};
    }
    if (op == "undo") {
        if (!engine.Undo()) {
            return {false, "stack empty"};
        }
        return {true, ""};
    }
    if (op == "lz-analyze") {
        FLAGS_lizzie = true;
        return { true, "" };
    }
    LOG(ERROR) << "invalid op: " << op;
    return {false, "unknown command"};
}

void GTPServing(std::istream &in, std::ostream &out)
{
    auto engine = InitEngine(FLAGS_config_path);
    if (FLAGS_init_moves.size()) {
        engine->Reset(FLAGS_init_moves);
    }
    std::cerr << std::flush;
    g_eval_task_queue.Set(g_config->eval_task_queue_size());
    //Eval_Routine* elva_batch_thread = new Eval_Routine(*engine);
    Eval_Routine* elva_batch_thread = new Eval_Routine();
    elva_batch_thread->Init();
    LOG(INFO) << "MCTSEngine: waiting all eval threads init";
    g_eval_threads_init_wg.Wait();
    LOG(INFO) << "MCTSEngine: all eval threads init done";
    engine->Init();
    if (FLAGS_lizzie)
    {
        g_analyze_thread = std::thread(&MCTSEngine::analyzes, engine.get());
        FLAGS_lizzie = false;
    }
    int id;
    bool has_id, succ;
    std::string cmd, output;
    while (std::getline(in, cmd)) {
        for (char &c: cmd) c = std::tolower(c);
        LOG(INFO) << "input cmd: " << cmd;
        google::FlushLogFiles(google::GLOG_INFO);

        std::istringstream ss(cmd);
        if ((has_id = (bool)(ss >> id))) {
            std::getline(ss, cmd);
        }

        std::tie(succ, output) = GTPExecute(*engine, cmd);
        LOG(INFO) << "?="[succ] << " " << output;

        out << "?="[succ];
        if (has_id) {
            out << id;
        }
        if (output.size()) {
            out << " " << output;
        }
        out << std::endl << std::endl;

        google::FlushLogFiles(google::GLOG_INFO);

        if (cmd.find("quit") != std::string::npos) {
            break;
        }
    }
    LOG(WARNING) << "exiting gtp serving";
}

void GenMoveOnce()
{
    auto engine = InitEngine(FLAGS_config_path);
    if (FLAGS_init_moves.size()) {
        engine->Reset(FLAGS_init_moves);
    }
    g_eval_task_queue.Set(g_config->eval_task_queue_size());
    //Eval_Routine* elva_batch_thread = new Eval_Routine(*engine);
    Eval_Routine* elva_batch_thread = new Eval_Routine();
    elva_batch_thread->Init();
    LOG(INFO) << "MCTSEngine: waiting all eval threads init";
    g_eval_threads_init_wg.Wait();
    LOG(INFO) << "MCTSEngine: all eval threads init done";
    engine->Init();
    GoCoordId x, y;
    engine->GenMove(x, y);
    engine->Move(x, y);
    printf("{\"Px\":%d, \"Py\":%d, \"Result\":0, \"Board\":\"\", \"Debug\":\"%s\"}",
           (int)x, (int)y, engine->GetDebugger().GetLastMoveDebugStr().c_str());
}

void GTPServingOnPort(int port)
{
    asio::io_service io_service;
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    asio::ip::tcp::acceptor acceptor(io_service, endpoint);

    auto allow_ip = FLAGS_allow_ip.size() ? SplitStr(FLAGS_allow_ip, ',') : std::vector<std::string>();
    for (;;) {
        LOG(INFO) << "waiting connection";

        asio::ip::tcp::iostream stream;
        asio::ip::tcp::endpoint peer;
        acceptor.accept(*stream.rdbuf(), peer);

        std::string ip = peer.address().to_string();
        LOG(INFO) << "accept " << ip;
        if (allow_ip.size() && std::find(allow_ip.begin(), allow_ip.end(), ip) == allow_ip.end()) {
            LOG(ERROR) << "client ip " << ip << " not in whitelist, reject";
            continue;
        }

#if defined(_WIN32) || defined(_WIN64)
        stream.rdbuf()->set_option(asio::ip::tcp::socket::keep_alive(true));
        GTPServing(stream, stream);
#else
        if (FLAGS_fork_per_request) {
            int pid, fork_cnt;
            for (fork_cnt = 0; fork_cnt < 2; ++fork_cnt) {
                io_service.notify_fork(asio::io_service::fork_prepare);
                PCHECK((pid = fork()) >= 0);
                if (pid == 0) {
                    io_service.notify_fork(asio::io_service::fork_child);
                } else {
                    io_service.notify_fork(asio::io_service::fork_parent);
                    break;
                }
            }
            if (fork_cnt > 0) {
                if (fork_cnt > 1) {
                    acceptor.close();
                    stream.rdbuf()->set_option(asio::ip::tcp::socket::keep_alive(true));
                    GTPServing(stream, stream);
                }
                exit(0);
            }
            PCHECK(waitpid(pid, NULL, 0) == pid);
        } else {
            stream.rdbuf()->set_option(asio::ip::tcp::socket::keep_alive(true));
            GTPServing(stream, stream);
        }
#endif
    }
}
struct myclass {           // function object type:
    int operator() (std::unique_ptr<MCTSEngine> x) { if (x->GetStatus()) return 1; else return 0; }
} myobject;


int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    if (FLAGS_gtp) 
    {
        if (FLAGS_listen_port == 0) {
            GTPServing(std::cin, std::cout);
        } else {
            GTPServingOnPort(FLAGS_listen_port);
        }
    } 
    else if (FLAGS_selfplay)
    {
        int c_count = 0;
        int single_thread = FLAGS_games_num / FLAGS_selfplay_thread_num;
        InitConfig(FLAGS_config_path);
        g_eval_task_queue.Set(g_config->eval_task_queue_size());
        Eval_Routine* elva_batch_thread = new Eval_Routine();
        elva_batch_thread->InitSelfplay();
        LOG(INFO) << "MCTSEngine: waiting all eval threads init";
        g_eval_threads_init_wg.Wait();
        LOG(INFO) << "MCTSEngine: all eval threads init done";
        std::vector<std::unique_ptr<MCTSEngine>> engines;

        for (int i = 0; i < FLAGS_selfplay_thread_num; ++i)
        {
            engines.emplace_back(new MCTSEngine(*g_config, i));
            engines.back()->InitSelfplay(single_thread);
        }
        for (; ; )
        {
            c_count = 0;
            for (int i = 0; i < FLAGS_selfplay_thread_num; ++i)
            {
                if (engines.at(i)->GetStatus())
                {
                    ++c_count;
                }
            }
            if (c_count == FLAGS_selfplay_thread_num)
                break;
        }
    }
    else 
    {
        GenMoveOnce();
    }
    //close all Thread
    if (g_analyze_thread.joinable())
    {
        LOG(WARNING) << "search apply threads: terminate";
        g_analyze_thread.join();
    }
    g_eval_task_queue.Close();
    LOG(INFO) << "main:: Waiting eval threads terminate";
    for (auto& th : g_eval_with_batch_threads) {
        th.join();
    }
}
