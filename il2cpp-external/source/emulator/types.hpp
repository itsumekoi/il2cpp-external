#pragma once
#include <Windows.h>
#include <cstdio>
#include <array>

enum class operand_type : std::uint8_t
{
	none,
	reg,
	xmm,
	mem,
	imm,
};

enum class operand_size : std::uint8_t
{
	byte = 1,
	word = 2,
	dword = 4,
	qword = 8,
	oword = 16,
	yword = 32,
};

enum class instruction_type : std::uint16_t
{
	unknown,

	mov,
	movzx,
	movsx,
	movsxd,
	lea,
	push,
	pushfq,
	pop,
	popfq,
	xchg,
	movbe,

	add,
	adc,
	sub,
	sbb,
	mul,
	imul,
	div,
	idiv,
	inc,
	dec,
	neg,
	cmp,
	test,

	and_instr,
	or_instr,
	xor_instr,
	not_instr,
	shl,
	shr,
	sar,
	rol,
	ror,
	rcl,
	rcr,
	bt,
	bts,
	btr,
	btc,
	bsf,
	bsr,
	bswap,
	lzcnt,
	tzcnt,
	popcnt,

	call,
	ret,
	jmp,

	je, jne, jg, jge, jl, jle,
	ja, jae, jb, jbe,
	jz, jnz, js, jns, jo, jno, jp, jnp,
	jcxz, jecxz, jrcxz,

	cmove, cmovne, cmovg, cmovge, cmovl, cmovle,
	cmova, cmovae, cmovb, cmovbe, cmpxchg,
	cmovs, cmovns, cmovo, cmovno,
	cmpxchg8b, cmpxchg16b,
	xadd,

	sete, setne, setg, setge, setl, setle,
	seta, setae, setb, setbe,
	sets, setns, seto, setno,

	movss, movsd,
	addss, addsd,
	subss, subsd,
	mulss, mulsd,
	divss, divsd,
	sqrtss, sqrtsd,
	minss, minsd,
	maxss, maxsd,
	rcpss,
	rsqrtss,
	cmpss, cmpsd,
	ucomiss, ucomisd,
	comiss, comisd,

	addps, addpd,
	subps, subpd,
	mulps, mulpd,
	divps, divpd,
	sqrtps, sqrtpd,
	minps, minpd,
	maxps, maxpd,
	rcpps,
	rsqrtps,
	cmpps, cmppd,
	dpps, dppd,
	haddps, haddpd,
	hsubps, hsubpd,

	paddb, paddw, paddd, paddq,
	psubb, psubw, psubd, psubq,
	pmullw, pmulld, pmulhw, pmulhuw,
	pmuludq, pmuldq,
	pavgb, pavgw,
	pminb, pminw, pmind, pminud, pminuw, pminub,
	pmaxb, pmaxw, pmaxd, pmaxud, pmaxuw, pmaxub,
	pcmpeqb, pcmpeqw, pcmpeqd, pcmpeqq,
	pcmpgtb, pcmpgtw, pcmpgtd, pcmpgtq,
	pand, pandn, por, pxor,
	psllw, pslld, psllq, psrld, psrlw, psrlq, psrad, psraw,
	pslldq, psrldq,
	pabsb, pabsw, pabsd,
	psadbw,

	cvtdq2ps, cvtps2dq, cvttps2dq,
	cvtdq2pd, cvtpd2dq, cvttpd2dq,
	cvtps2pd, cvtpd2ps,

	cvtsi2ss, cvtsi2sd,
	cvtss2sd, cvtsd2ss,
	vcvtss2sd, vcvtsd2ss,
	cvttss2si, cvttsd2si,
	cvtss2si, cvtsd2si,

	movaps, movups,
	movapd, movupd,
	movdqa, movdqu,
	movq,
	movd,
	movlps, movhps,
	movlpd, movhpd,
	movmskps, movmskpd,
	movntps, movntpd, movntdq, movnti,
	shufps, shufpd,
	shufpd128,
	unpcklps, unpckhps,
	unpcklpd, unpckhpd,
	punpcklbw, punpcklwd, punpckldq, punpcklqdq,
	punpckhbw, punpckhwd, punpckhdq, punpckhqdq,
	packuswb, packusdw, packsswb, packssdw,
	pshufb, pshufd, pshufhw, pshuflw,
	palignr,
	pblendw, blendps, blendpd,
	pblendvb, blendvps, blendvpd,
	insertps,
	pinsrb, pinsrw, pinsrd, pinsrq,
	pextrb, pextrw, pextrd, pextrq,
	pmovzxbw, pmovzxbd, pmovzxbq,
	pmovzxwd, pmovzxwq, pmovzxdq,
	pmovsxbw, pmovsxbd, pmovsxbq,
	pmovsxwd, pmovsxwq, pmovsxdq,
	pmovmskb,

	xorps, xorpd,
	andps, andpd,
	andnps, andnpd,
	orps, orpd,

	ldmxcsr, stmxcsr,
	fstcw, fldcw,
	sfence, lfence, mfence,
	pause,
	prefetchnta, prefetcht0, prefetcht1, prefetcht2,
	clflush,

	vzeroupper,
	vzeroall,
	vinsertf128, vinserti128,
	vextractf128, vextracti128,
	vbroadcastss, vbroadcastsd, vbroadcastf128,
	vpbroadcastb, vpbroadcastw, vpbroadcastd, vpbroadcastq,
	vpxor,
	vpcmpeqb, vpcmpeqw, vpcmpeqd,
	vpmovmskb,

	vmovdqu256, vmovdqa256,
	vmovups256, vmovupd256,
	vmovaps256, vmovapd256,
	vperm2f128, vperm2i128,
	vpermd, vpermps, vpermq, vpermpd,
	vpermilps, vpermilpd,

	nop,
	rdtsc,
	rdtscp,
	cpuid,
	syscall,
	sysret,
	int_instr,
	cdq,
	cqo,
	cbw,
	cwde,
	cdqe,
	lahf,
	sahf,
	pushf,
	popf,
	std_instr,
	cld,
	clc,
	stc,
	cmc,
	ud2,
	hlt,

	vpshufb,
};

struct memory_operand
{
	std::uint8_t base;
	std::uint8_t index;
	std::uint8_t scale;
	std::int32_t displacement;

	memory_operand ( )
		: base ( 0xFF ), index ( 0xFF ), scale ( 1 ), displacement ( 0 )
	{ }
};

struct operand
{
	operand_type type;
	operand_size size;

	union
	{
		std::uint8_t reg;
		std::uint8_t xmm;
		memory_operand mem;
		std::uint64_t imm;
	};

	operand ( )
		: type ( operand_type::none ), size ( operand_size::qword ), imm ( 0 )
	{ }
};

struct instruction
{
	instruction_type type;
	std::uint8_t opcode;
	std::uint8_t length;
	std::uint64_t address;

	operand operands[ 3 ];
	std::uint8_t operand_count;

	bool has_rex;
	std::uint8_t rex;

	bool has_operand_size;
	bool has_address_size;
	bool has_rep;
	bool has_lock;
	bool has_repne;

	bool rex_w;
	bool rex_r;
	bool rex_x;
	bool rex_b;

	std::uint8_t modrm_byte;
	bool has_modrm;

	bool has_vex;
	bool vex_r;
	bool vex_x;
	bool vex_b;
	bool vex_w;
	bool vex_l;
	std::uint8_t vex_vvvv;
	std::uint8_t vex_pp;
	std::uint8_t vex_map;

	bool has_segment_prefix;
	std::uint8_t segment_prefix;

	instruction ( )
		: type ( instruction_type::unknown ), opcode ( 0 ), length ( 0 ), address ( 0 ),
		operand_count ( 0 ),
		has_rex ( false ), rex ( 0 ),
		has_operand_size ( false ), has_address_size ( false ),
		has_rep ( false ), has_repne ( false ),
		rex_w ( false ), rex_r ( false ), rex_x ( false ), rex_b ( false ),
		modrm_byte ( 0 ), has_modrm ( false ),
		has_vex ( false ), vex_r ( false ), vex_x ( false ), vex_b ( false ), vex_w ( false ), vex_l ( false ),
		vex_vvvv ( 0 ), vex_pp ( 0 ), vex_map ( 0 ),
		has_segment_prefix ( false ), segment_prefix ( 0 )
	{ }
};