{
	"skb->sk: no NULL check",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_1, 0),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid mem access 'sock_common_or_null'",
},
{
	"skb->sk: sk->family [non fullsock field]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_1, offsetof(struct bpf_sock, family)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"skb->sk: sk->type [fullsock field]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_1, offsetof(struct bpf_sock, type)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid sock_common access",
},
{
	"bpf_sk_fullsock(skb->sk): no !skb->sk check",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "type=sock_common_or_null expected=sock_common",
},
{
	"sk_fullsock(skb->sk): no NULL check on ret",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, type)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid mem access 'sock_or_null'",
},
{
	"sk_fullsock(skb->sk): sk->type [fullsock field]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, type)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->family [non fullsock field]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, family)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->state [narrow load]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_B, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, state)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->dst_port [narrow load]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_H, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, dst_port)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->dst_port [load 2nd byte]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_B, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, dst_port) + 1),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid sock access",
},
{
	"sk_fullsock(skb->sk): sk->dst_ip6 [load 2nd byte]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_B, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, dst_ip6[0]) + 1),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->type [narrow load]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_B, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, type)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): sk->protocol [narrow load]",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_B, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, protocol)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"sk_fullsock(skb->sk): beyond last field",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetofend(struct bpf_sock, state)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid sock access",
},
{
	"bpf_tcp_sock(skb->sk): no !skb->sk check",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "type=sock_common_or_null expected=sock_common",
},
{
	"bpf_tcp_sock(skb->sk): no NULL check on ret",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_tcp_sock, snd_cwnd)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid mem access 'tcp_sock_or_null'",
},
{
	"bpf_tcp_sock(skb->sk): tp->snd_cwnd",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_tcp_sock, snd_cwnd)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"bpf_tcp_sock(skb->sk): tp->bytes_acked",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_tcp_sock, bytes_acked)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"bpf_tcp_sock(skb->sk): beyond last field",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, offsetofend(struct bpf_tcp_sock, bytes_acked)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = REJECT,
	.errstr = "invalid tcp_sock access",
},
{
	"bpf_tcp_sock(bpf_sk_fullsock(skb->sk)): tp->snd_cwnd",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_tcp_sock, snd_cwnd)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_CGROUP_SKB,
	.result = ACCEPT,
},
{
	"bpf_sk_release(skb->sk)",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JEQ, BPF_REG_1, 0, 1),
	BPF_EMIT_CALL(BPF_FUNC_sk_release),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "reference has not been acquired before",
},
{
	"bpf_sk_release(bpf_sk_fullsock(skb->sk))",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
	BPF_EMIT_CALL(BPF_FUNC_sk_release),
	BPF_MOV64_IMM(BPF_REG_0, 1),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "reference has not been acquired before",
},
{
	"bpf_sk_release(bpf_tcp_sock(skb->sk))",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_tcp_sock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
	BPF_EMIT_CALL(BPF_FUNC_sk_release),
	BPF_MOV64_IMM(BPF_REG_0, 1),
	BPF_EXIT_INSN(),
	},
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "reference has not been acquired before",
},
{
	"sk_storage_get(map, skb->sk, NULL, 0): value == NULL",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_MOV64_IMM(BPF_REG_4, 0),
	BPF_MOV64_IMM(BPF_REG_3, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_0),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_sk_storage_get),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_sk_storage_map = { 11 },
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = ACCEPT,
},
{
	"sk_storage_get(map, skb->sk, 1, 1): value == 1",
	.insns = {
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_MOV64_IMM(BPF_REG_4, 1),
	BPF_MOV64_IMM(BPF_REG_3, 1),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_0),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_sk_storage_get),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_sk_storage_map = { 11 },
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "R3 type=inv expected=fp",
},
{
	"sk_storage_get(map, skb->sk, &stack_value, 1): stack_value",
	.insns = {
	BPF_MOV64_IMM(BPF_REG_2, 0),
	BPF_STX_MEM(BPF_DW, BPF_REG_10, BPF_REG_2, -8),
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_MOV64_IMM(BPF_REG_4, 1),
	BPF_MOV64_REG(BPF_REG_3, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_3, -8),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_0),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_sk_storage_get),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_sk_storage_map = { 14 },
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = ACCEPT,
},
{
	"sk_storage_get(map, skb->sk, &stack_value, 1): partially init stack_value",
	.insns = {
	BPF_MOV64_IMM(BPF_REG_2, 0),
	BPF_STX_MEM(BPF_W, BPF_REG_10, BPF_REG_2, -8),
	BPF_LDX_MEM(BPF_DW, BPF_REG_1, BPF_REG_1, offsetof(struct __sk_buff, sk)),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_1, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_EMIT_CALL(BPF_FUNC_sk_fullsock),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 2),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	BPF_MOV64_IMM(BPF_REG_4, 1),
	BPF_MOV64_REG(BPF_REG_3, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_3, -8),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_0),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_sk_storage_get),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_sk_storage_map = { 14 },
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "invalid indirect read from stack",
},
{
	"bpf_map_lookup_elem(smap, &key)",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_sk_storage_map = { 3 },
	.prog_type = BPF_PROG_TYPE_SCHED_CLS,
	.result = REJECT,
	.errstr = "cannot pass map_type 24 into func bpf_map_lookup_elem",
},
{
	"bpf_map_lookup_elem(xskmap, &key); xs->queue_id",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -8, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -8),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_xdp_sock, queue_id)),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_map_xskmap = { 3 },
	.prog_type = BPF_PROG_TYPE_XDP,
	.result = ACCEPT,
},
{
	"bpf_map_lookup_elem(sockmap, &key)",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_map_sockmap = { 3 },
	.prog_type = BPF_PROG_TYPE_SK_SKB,
	.result = REJECT,
	.errstr = "Unreleased reference id=2 alloc_insn=5",
},
{
	"bpf_map_lookup_elem(sockhash, &key)",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_map_sockhash = { 3 },
	.prog_type = BPF_PROG_TYPE_SK_SKB,
	.result = REJECT,
	.errstr = "Unreleased reference id=2 alloc_insn=5",
},
{
	"bpf_map_lookup_elem(sockmap, &key); sk->type [fullsock field]; bpf_sk_release(sk)",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, type)),
	BPF_EMIT_CALL(BPF_FUNC_sk_release),
	BPF_EXIT_INSN(),
	},
	.fixup_map_sockmap = { 3 },
	.prog_type = BPF_PROG_TYPE_SK_SKB,
	.result = ACCEPT,
},
{
	"bpf_map_lookup_elem(sockhash, &key); sk->type [fullsock field]; bpf_sk_release(sk)",
	.insns = {
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, 0),
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),
	BPF_EXIT_INSN(),
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, offsetof(struct bpf_sock, type)),
	BPF_EMIT_CALL(BPF_FUNC_sk_release),
	BPF_EXIT_INSN(),
	},
	.fixup_map_sockhash = { 3 },
	.prog_type = BPF_PROG_TYPE_SK_SKB,
	.result = ACCEPT,
},
