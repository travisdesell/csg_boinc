// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include "common_defs.h"
#include "diagnostics.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"

#include "cc_config.h"

using std::string;

LOG_FLAGS::LOG_FLAGS() {
    init();
}

void LOG_FLAGS::init() {
    memset(this, 0, sizeof(LOG_FLAGS));
    // on by default (others are off by default)
    //
    task = true;
    file_xfer = true;
    sched_ops = true;
}

// Parse log flag preferences
//
int LOG_FLAGS::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, "/log_flags")) return 0;
        if (xp.parse_bool(tag, "file_xfer", file_xfer)) continue;
        if (xp.parse_bool(tag, "sched_ops", sched_ops)) continue;
        if (xp.parse_bool(tag, "task", task)) continue;

        if (xp.parse_bool(tag, "app_msg_receive", app_msg_receive)) continue;
        if (xp.parse_bool(tag, "app_msg_send", app_msg_send)) continue;
        if (xp.parse_bool(tag, "benchmark_debug", benchmark_debug)) continue;
        if (xp.parse_bool(tag, "checkpoint_debug", checkpoint_debug)) continue;
        if (xp.parse_bool(tag, "coproc_debug", coproc_debug)) continue;
        if (xp.parse_bool(tag, "cpu_sched", cpu_sched)) continue;
        if (xp.parse_bool(tag, "cpu_sched_debug", cpu_sched_debug)) continue;
        if (xp.parse_bool(tag, "cpu_sched_status", cpu_sched_status)) continue;
        if (xp.parse_bool(tag, "dcf_debug", dcf_debug)) continue;
        if (xp.parse_bool(tag, "debt_debug", debt_debug)) continue;
        if (xp.parse_bool(tag, "std_debug", std_debug)) continue;
        if (xp.parse_bool(tag, "file_xfer_debug", file_xfer_debug)) continue;
        if (xp.parse_bool(tag, "gui_rpc_debug", gui_rpc_debug)) continue;
        if (xp.parse_bool(tag, "heartbeat_debug", heartbeat_debug)) continue;
        if (xp.parse_bool(tag, "http_debug", http_debug)) continue;
        if (xp.parse_bool(tag, "http_xfer_debug", http_xfer_debug)) continue;
        if (xp.parse_bool(tag, "mem_usage_debug", mem_usage_debug)) continue;
        if (xp.parse_bool(tag, "network_status_debug", network_status_debug)) continue;
        if (xp.parse_bool(tag, "poll_debug", poll_debug)) continue;
        if (xp.parse_bool(tag, "proxy_debug", proxy_debug)) continue;
        if (xp.parse_bool(tag, "rr_simulation", rr_simulation)) continue;
        if (xp.parse_bool(tag, "sched_op_debug", sched_op_debug)) continue;
        if (xp.parse_bool(tag, "scrsave_debug", scrsave_debug)) continue;
        if (xp.parse_bool(tag, "slot_debug", slot_debug)) continue;
        if (xp.parse_bool(tag, "state_debug", state_debug)) continue;
        if (xp.parse_bool(tag, "statefile_debug", statefile_debug)) continue;
        if (xp.parse_bool(tag, "task_debug", task_debug)) continue;
        if (xp.parse_bool(tag, "time_debug", time_debug)) continue;
        if (xp.parse_bool(tag, "unparsed_xml", unparsed_xml)) continue;
        if (xp.parse_bool(tag, "work_fetch_debug", work_fetch_debug)) continue;
        if (xp.parse_bool(tag, "notice_debug", notice_debug)) continue;
        xp.skip_unexpected(tag, true, "LOG_FLAGS::parse");
    }
    return ERR_XML_PARSE;
}

int LOG_FLAGS::write(MIOFILE& out) {
    out.printf(
        "    <log_flags>\n"
        "        <file_xfer>%d</file_xfer>\n"
        "        <sched_ops>%d</sched_ops>\n"
        "        <task>%d</task>\n"
        "        <app_msg_receive>%d</app_msg_receive>\n"
        "        <app_msg_send>%d</app_msg_send>\n"
        "        <benchmark_debug>%d</benchmark_debug>\n"
        "        <checkpoint_debug>%d</checkpoint_debug>\n"
        "        <coproc_debug>%d</coproc_debug>\n"
        "        <cpu_sched>%d</cpu_sched>\n"
        "        <cpu_sched_debug>%d</cpu_sched_debug>\n"
        "        <cpu_sched_status>%d</cpu_sched_status>\n"
        "        <dcf_debug>%d</dcf_debug>\n"
        "        <debt_debug>%d</debt_debug>\n"
        "        <file_xfer_debug>%d</file_xfer_debug>\n"
        "        <gui_rpc_debug>%d</gui_rpc_debug>\n"
        "        <heartbeat_debug>%d</heartbeat_debug>\n"
        "        <http_debug>%d</http_debug>\n"
        "        <http_xfer_debug>%d</http_xfer_debug>\n"
        "        <mem_usage_debug>%d</mem_usage_debug>\n"
        "        <network_status_debug>%d</network_status_debug>\n"
        "        <poll_debug>%d</poll_debug>\n"
        "        <proxy_debug>%d</proxy_debug>\n"
        "        <rr_simulation>%d</rr_simulation>\n"
        "        <sched_op_debug>%d</sched_op_debug>\n"
        "        <scrsave_debug>%d</scrsave_debug>\n"
        "        <slot_debug>%d</slot_debug>\n"
        "        <state_debug>%d</state_debug>\n"
        "        <statefile_debug>%d</statefile_debug>\n"
        "        <std_debug>%d</std_debug>\n"
        "        <task_debug>%d</task_debug>\n"
        "        <time_debug>%d</time_debug>\n"
        "        <unparsed_xml>%d</unparsed_xml>\n"
        "        <work_fetch_debug>%d</work_fetch_debug>\n"
        "        <notice_debug>%d</notice_debug>\n"
        "    </log_flags>\n",
        file_xfer ? 1 : 0,
        sched_ops ? 1 : 0,
        task ? 1 : 0,
        app_msg_receive ? 1 : 0,
        app_msg_send ? 1 : 0,
        benchmark_debug ? 1 : 0,
        checkpoint_debug  ? 1 : 0,
        coproc_debug ? 1 : 0,
        cpu_sched ? 1 : 0,
        cpu_sched_debug ? 1 : 0,
        cpu_sched_status ? 1 : 0,
        dcf_debug ? 1 : 0,
        debt_debug ? 1 : 0,
        file_xfer_debug ? 1 : 0,
        gui_rpc_debug ? 1 : 0,
        heartbeat_debug ? 1 : 0,
        http_debug ? 1 : 0,
        http_xfer_debug ? 1 : 0,
        mem_usage_debug ? 1 : 0,
        network_status_debug ? 1 : 0,
        poll_debug ? 1 : 0,
        proxy_debug ? 1 : 0,
        rr_simulation ? 1 : 0,
        sched_op_debug ? 1 : 0,
        scrsave_debug ? 1 : 0,
        slot_debug ? 1 : 0,
        state_debug ? 1 : 0,
        statefile_debug ? 1 : 0,
        std_debug ? 1 : 0,
        task_debug ? 1 : 0,
        time_debug ? 1 : 0,
        unparsed_xml ? 1 : 0,
        work_fetch_debug ? 1 : 0,
        notice_debug ? 1 : 0
    );
    
    return 0;
}

CONFIG::CONFIG() {
}

// this is called first thing by client
//
void CONFIG::defaults() {
    abort_jobs_on_exit = false;
    allow_multiple_clients = false;
    allow_remote_gui_rpc = false;
    alt_platforms.clear();
    client_version_check_url = "http://boinc.berkeley.edu/download.php?xml=1";
    client_download_url = "http://boinc.berkeley.edu/download.php";
    disallow_attach = false;
    dont_check_file_sizes = false;
    dont_contact_ref_site = false;
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    exit_after_finish = false;
    exit_when_idle = false;
    fetch_minimal_work = false;
    force_auth = "default";
    http_1_0 = false;
    ignore_cuda_dev.clear();
    ignore_ati_dev.clear();
    max_file_xfers = MAX_FILE_XFERS;
    max_file_xfers_per_project = MAX_FILE_XFERS_PER_PROJECT;
    max_stderr_file_size = 0;
    max_stdout_file_size = 0;
    max_tasks_reported = 0;
    ncpus = -1;
    network_test_url = "http://www.google.com/";
    no_alt_platform = false;
    no_gpus = false;
    no_info_fetch = false;
    no_priority_change = false;
    os_random_only = false;
    proxy_info.clear();
    report_results_immediately = false;
    run_apps_manually = false;
    save_stats_days = 30;
    simple_gui_only = false;
    skip_cpu_benchmarks = false;
    start_delay = 0;
    stderr_head = false;
    suppress_net_info = false;
    unsigned_apps_ok = false;
    use_all_gpus = false;
    use_certs = false;
    use_certs_only = false;
    zero_debts = false;
}


int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;
    string s;
    int n, retval;

    //clear();
    // don't do this here because some options are set by cmdline args,
    // which are parsed first
    // but do clear these, which aren't accessable via cmdline:
    //
    alt_platforms.clear();
    exclusive_apps.clear();
    exclusive_gpu_apps.clear();
    ignore_cuda_dev.clear();
    ignore_ati_dev.clear();

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, "/options")) {
            return 0;
        }
        if (xp.parse_bool(tag, "abort_jobs_on_exit", abort_jobs_on_exit)) continue;
        if (xp.parse_bool(tag, "allow_multiple_clients", allow_multiple_clients)) continue;
        if (xp.parse_bool(tag, "allow_remote_gui_rpc", allow_remote_gui_rpc)) continue;
        if (xp.parse_string(tag, "alt_platform", s)) {
            alt_platforms.push_back(s);
            continue;
        }
        if (xp.parse_string(tag, "client_download_url", client_download_url)) {
            downcase_string(client_download_url);
            continue;
        }
        if (xp.parse_string(tag, "client_version_check_url", client_version_check_url)) {
            downcase_string(client_version_check_url);
            continue;
        }
        if (!strcmp(tag, "coproc")) {
            COPROC c;
            retval = c.parse(xp);
            if (retval) return retval;
            retval = config_coprocs.add(c);
            continue;
        }
        if (xp.parse_str(tag, "data_dir", data_dir, sizeof(data_dir))) {
            continue;
        }
        if (xp.parse_bool(tag, "disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool(tag, "dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_string(tag, "exclusive_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_string(tag, "exclusive_gpu_app", s)) {
            if (!strstr(s.c_str(), "boinc")) {
                exclusive_gpu_apps.push_back(s);
            }
            continue;
        }
        if (xp.parse_bool(tag, "exit_after_finish", exit_after_finish)) continue;
        if (xp.parse_bool(tag, "exit_when_idle", exit_when_idle)) {
            if (exit_when_idle) {
                report_results_immediately = true;
            }
            continue;
        }
        if (xp.parse_bool(tag, "fetch_minimal_work", fetch_minimal_work)) continue;
        if (xp.parse_string(tag, "force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool(tag, "http_1_0", http_1_0)) continue;
        if (xp.parse_int(tag, "ignore_cuda_dev", n)) {
            ignore_cuda_dev.push_back(n);
            continue;
        }
        if (xp.parse_int(tag, "ignore_ati_dev", n)) {
            ignore_ati_dev.push_back(n);
            continue;
        }
        if (xp.parse_int(tag, "max_file_xfers", max_file_xfers)) continue;
        if (xp.parse_int(tag, "max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        if (xp.parse_int(tag, "max_stderr_file_size", max_stderr_file_size)) continue;
        if (xp.parse_int(tag, "max_stdout_file_size", max_stdout_file_size)) continue;
        if (xp.parse_int(tag, "max_tasks_reported", max_tasks_reported)) continue;
        if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        if (xp.parse_string(tag, "network_test_url", network_test_url)) {
            downcase_string(network_test_url);
            continue;
        }
        if (xp.parse_bool(tag, "no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool(tag, "no_gpus", no_gpus)) continue;
        if (xp.parse_bool(tag, "no_info_fetch", no_info_fetch)) continue;
        if (xp.parse_bool(tag, "no_priority_change", no_priority_change)) continue;
        if (xp.parse_bool(tag, "os_random_only", os_random_only)) continue;
#ifndef SIM
        if (!strcmp(tag, "proxy_info")) {
            int retval = proxy_info.parse_config(*xp.f);
            if (retval) return retval;
            continue;
        }
#endif
        if (xp.parse_bool(tag, "report_results_immediately", report_results_immediately)) continue;
        if (xp.parse_bool(tag, "run_apps_manually", run_apps_manually)) continue;
        if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        if (xp.parse_bool(tag, "simple_gui_only", simple_gui_only)) continue;
        if (xp.parse_bool(tag, "skip_cpu_benchmarks", skip_cpu_benchmarks)) continue;
        if (xp.parse_double(tag, "start_delay", start_delay)) continue;
        if (xp.parse_bool(tag, "stderr_head", stderr_head)) continue;
        if (xp.parse_bool(tag, "suppress_net_info", suppress_net_info)) continue;
        if (xp.parse_bool(tag, "unsigned_apps_ok", unsigned_apps_ok)) continue;
        if (xp.parse_bool(tag, "use_all_gpus", use_all_gpus)) continue;
        if (xp.parse_bool(tag, "use_certs", use_certs)) continue;
        if (xp.parse_bool(tag, "use_certs_only", use_certs_only)) continue;
        if (xp.parse_bool(tag, "zero_debts", zero_debts)) continue;

        xp.skip_unexpected(tag, true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(MIOFILE& in, LOG_FLAGS& log_flags) {
    char tag[256];
    XML_PARSER xp(&in);
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (!strcmp(tag, "/cc_config")) return 0;
        if (!strcmp(tag, "log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (!strcmp(tag, "options")) {
            parse_options(xp);
            continue;
        }
        if (!strcmp(tag, "options/")) continue;
        if (!strcmp(tag, "log_flags/")) continue;
    }
    return ERR_XML_PARSE;
}

int CONFIG::write(MIOFILE& out, LOG_FLAGS& log_flags) {
    unsigned int i;
    int j;

    out.printf("<set_cc_config>\n");
    out.printf("<cc_config>\n");

    log_flags.write(out);

    out.printf(
        "    <options>\n"
        "        <abort_jobs_on_exit>%d</abort_jobs_on_exit>\n"
        "        <allow_multiple_clients>%d</allow_multiple_clients>\n"
        "        <allow_remote_gui_rpc>%d</allow_remote_gui_rpc>\n",
        abort_jobs_on_exit ? 1 : 0,
        allow_multiple_clients ? 1 : 0,
        allow_remote_gui_rpc ? 1 : 0
    );

    for (i=0; i<alt_platforms.size(); ++i) {
        out.printf(
            "        <alt_platform>%s</alt_platform>\n",
            alt_platforms[i].c_str()
        );
    }
    
    out.printf(
        "        <client_version_check_url>%s</client_version_check_url>\n"
        "        <client_download_url>%s</client_download_url>\n",
        client_version_check_url.c_str(),
        client_download_url.c_str()
    );
    
    for (i=0; i<(unsigned int)config_coprocs.n_rsc; ++i) {
        out.printf(
            "        <coproc>"
            "            <type>%s</type>\n"
            "            <count>%d</count>\n"
            "            <peak_flops>%f</peak_flops>\n"
            "            <device_nums>",
            config_coprocs.coprocs[i].type,
            config_coprocs.coprocs[i].count,
            config_coprocs.coprocs[i].peak_flops
        );
        for (j=0; j<config_coprocs.coprocs[i].count; ++i) {
            out.printf("%d", config_coprocs.coprocs[i].device_nums[j]);
            if (j < (config_coprocs.coprocs[i].count - 1)) {
                out.printf(" ");
            }
        }
        out.printf(
            "</device_nums>\n"
            "        </coproc>"
        );
    }
    
    out.printf(
        "        <data_dir>%s</data_dir>\n"
        "        <disallow_attach>%d</disallow_attach>\n"
        "        <dont_check_file_sizes>%d</dont_check_file_sizes>\n"
        "        <dont_contact_ref_site>%d</dont_contact_ref_site>\n",
        data_dir,
        disallow_attach,
        dont_check_file_sizes,
        dont_contact_ref_site
    );
    
    for (i=0; i<exclusive_apps.size(); ++i) {
        out.printf(
            "        <exclusive_app>%s</exclusive_app>\n",
            exclusive_apps[i].c_str()
        );
    }
            
    for (i=0; i<exclusive_apps.size(); ++i) {
        out.printf(
            "        <exclusive_gpu_app>%s</exclusive_gpu_app>\n",
            exclusive_gpu_apps[i].c_str()
        );
    }
            
    out.printf(
        "        <exit_after_finish>%d</exit_after_finish>\n"
        "        <exit_when_idle>%d</exit_when_idle>\n"
        "        <fetch_minimal_work>%d</fetch_minimal_work>\n"
        "        <force_auth>%s</force_auth>\n"
        "        <http_1_0>%d</http_1_0>\n",
        exit_after_finish,
        exit_when_idle,
        fetch_minimal_work,
        force_auth.c_str(),
        http_1_0
    );
        
    for (i=0; i<ignore_cuda_dev.size(); ++i) {
        out.printf(
            "        <ignore_cuda_dev>%d</ignore_cuda_dev>\n",
            ignore_cuda_dev[i]
        );
    }

    for (i=0; i<ignore_ati_dev.size(); ++i) {
        out.printf(
            "        <ignore_ati_dev>%d</ignore_ati_dev>\n",
            ignore_ati_dev[i]
        );
    }
        
    out.printf(
        "        <max_file_xfers>%d</max_file_xfers>\n"
        "        <max_file_xfers_per_project>%d</max_file_xfers_per_project>\n"
        "        <max_stderr_file_size>%d</max_stderr_file_size>\n"
        "        <max_stdout_file_size>%d</max_stdout_file_size>\n"
        "        <max_tasks_reported>%d</max_tasks_reported>\n"
        "        <ncpus>%d</ncpus>\n"
        "        <network_test_url>%s</network_test_url>\n"
        "        <no_alt_platform>%d</no_alt_platform>\n"
        "        <no_gpus>%d</no_gpus>\n"
        "        <no_info_fetch>%d</no_info_fetch>\n"
        "        <no_priority_change>%d</no_priority_change>\n"
        "        <os_random_only>%d</os_random_only>\n",
        max_file_xfers,
        max_file_xfers_per_project,
        max_stderr_file_size,
        max_stdout_file_size,
        max_tasks_reported,
        ncpus,
        network_test_url.c_str(),
        no_alt_platform,
        no_gpus,
        no_info_fetch,
        no_priority_change,
        os_random_only
    );
    
    proxy_info.write(out);
    
    out.printf(
        "        <report_results_immediately>%d</report_results_immediately>\n"
        "        <run_apps_manually>%d</run_apps_manually>\n"
        "        <save_stats_days>%d</save_stats_days>\n"
        "        <skip_cpu_benchmarks>%d</skip_cpu_benchmarks>\n"
        "        <simple_gui_only>%d</simple_gui_only>\n"
        "        <start_delay>%d</start_delay>\n"
        "        <stderr_head>%d</stderr_head>\n"
        "        <suppress_net_info>%d</suppress_net_info>\n"
        "        <unsigned_apps_ok>%d</unsigned_apps_ok>\n"
        "        <use_all_gpus>%d</use_all_gpus>\n"
        "        <use_certs>%d</use_certs>\n"
        "        <use_certs_only>%d</use_certs_only>\n"
        "        <zero_debts>%d</zero_debts>\n",
        report_results_immediately,
        run_apps_manually,
        save_stats_days,
        skip_cpu_benchmarks,
        simple_gui_only,
        start_delay,
        stderr_head,
        suppress_net_info,
        unsigned_apps_ok,
        use_all_gpus,
        use_certs,
        use_certs_only,
        zero_debts
    );

    out.printf("    </options>\n</cc_config>\n");
    out.printf("</set_cc_config>\n");
    return 0;
}