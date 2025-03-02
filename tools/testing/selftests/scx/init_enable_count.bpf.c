/* SPDX-License-Identifier: GPL-2.0 */
/*
 * A scheduler that verifies that we do proper counting of init, enable, etc
 * callbacks.
 *
 * Copyright (c) 2023 Meta Platforms, Inc. and affiliates.
 * Copyright (c) 2023 David Vernet <dvernet@meta.com>
 * Copyright (c) 2023 Tejun Heo <tj@kernel.org>
 */

#include <scx/common.bpf.h>

char _license[] SEC("license") = "GPL";

u64 init_task_cnt, exit_task_cnt, enable_cnt, disable_cnt;
volatile const bool switch_all;

s32 BPF_STRUCT_OPS_SLEEPABLE(cnt_init_task, struct task_struct *p,
			     struct scx_init_task_args *args)
{
	__sync_fetch_and_add(&init_task_cnt, 1);

	return 0;
}

void BPF_STRUCT_OPS(cnt_exit_task, struct task_struct *p)
{
	__sync_fetch_and_add(&exit_task_cnt, 1);
}

void BPF_STRUCT_OPS(cnt_enable, struct task_struct *p)
{
	__sync_fetch_and_add(&enable_cnt, 1);
}

void BPF_STRUCT_OPS(cnt_disable, struct task_struct *p)
{
	__sync_fetch_and_add(&disable_cnt, 1);
}

s32 BPF_STRUCT_OPS(cnt_init)
{
	if (switch_all)
		scx_bpf_switch_all();

	return 0;
}

SEC(".struct_ops.link")
struct sched_ext_ops init_enable_count_ops = {
	.init_task	= cnt_init_task,
	.exit_task	= cnt_exit_task,
	.enable		= cnt_enable,
	.disable	= cnt_disable,
	.init		= cnt_init,
	.name		= "init_enable_count",
};
