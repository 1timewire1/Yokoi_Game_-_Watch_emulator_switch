#include "tamago_cpu6502.h"
/* NOTE: RTS/RTI pop the low byte first in separate statements. "pop(c) | (pop(c) << 8)" has no defined
   evaluation order in C, so the return address could come out byte-swapped depending on the compiler. */
#include <string.h>

typedef enum { OP_INVALID=0, OP_ADC, OP_AND, OP_ASL, OP_BCC, OP_BCS, OP_BEQ, OP_BIT, OP_BMI, OP_BNE, OP_BPL, OP_BRK, OP_BVC, OP_BVS, OP_CLC, OP_CLD, OP_CLI, OP_CLV, OP_CMP, OP_CPX, OP_CPY, OP_DEC, OP_DEX, OP_DEY, OP_EOR, OP_INC, OP_INX, OP_INY, OP_JMP, OP_JSR, OP_LDA, OP_LDX, OP_LDY, OP_LSR, OP_NOP, OP_ORA, OP_PHA, OP_PHP, OP_PLA, OP_PLP, OP_ROL, OP_ROR, OP_RTI, OP_RTS, OP_SBC, OP_SEC, OP_SED, OP_SEI, OP_STA, OP_STX, OP_STY, OP_TAX, OP_TAY, OP_TSX, OP_TXA, OP_TXS, OP_TYA } Op;
typedef enum { AM_INVALID=0, AM_ABSOLUTE, AM_ABSOLUTEX, AM_ABSOLUTEY, AM_ACCUMULATOR, AM_IMMEDIATE, AM_IMPLIED, AM_INDIRECT, AM_INDIRECTX, AM_INDIRECTY, AM_RELATIVE, AM_ZEROPAGE, AM_ZEROPAGEX, AM_ZEROPAGEY } AddrMode;
typedef struct { Op op; AddrMode mode; uint8_t cycles; bool valid; } OpInfo;
#include "tamago_opcode_table.inc"

static uint8_t rd(TamagoCpu *c, uint16_t a) { return c->read ? c->read(c->bus_ctx,a) : 0xFF; }
static void wr(TamagoCpu *c, uint16_t a, uint8_t v) { if (c->write) c->write(c->bus_ctx,a,v); }
static uint8_t fetch(TamagoCpu *c) { uint8_t v=rd(c,c->pc); c->pc++; return v; }
static uint16_t fetch16(TamagoCpu *c) { uint8_t l=fetch(c), h=fetch(c); return (uint16_t)(l | ((uint16_t)h<<8)); }
static uint16_t read16(TamagoCpu *c,uint16_t a) { uint8_t l=rd(c,a),h=rd(c,(uint16_t)(a+1)); return (uint16_t)(l|((uint16_t)h<<8)); }
static void push(TamagoCpu*c,uint8_t v){wr(c,(uint16_t)(0x100|c->s),v);c->s--;}
static uint8_t pop(TamagoCpu*c){c->s++;return rd(c,(uint16_t)(0x100|c->s));}
static void nz(TamagoCpu*c,uint8_t v){c->z=(v==0);c->n=(v>>7)&1;}
uint8_t tamago_cpu_status(const TamagoCpu*c){return (uint8_t)(0x20|(c->c?1:0)|(c->z?2:0)|(c->i?4:0)|(c->d?8:0)|(c->v?0x40:0)|(c->n?0x80:0));}
void tamago_cpu_set_status(TamagoCpu*c,uint8_t p){c->c=p&1;c->z=(p>>1)&1;c->i=(p>>2)&1;c->d=(p>>3)&1;c->v=(p>>6)&1;c->n=(p>>7)&1;}
void tamago_cpu_init(TamagoCpu*c,void*ctx,tamago_read_fn r,tamago_write_fn w){memset(c,0,sizeof(*c));c->bus_ctx=ctx;c->read=r;c->write=w;c->s=0xFD;c->i=1;}
void tamago_cpu_reset(TamagoCpu*c){c->a=c->x=c->y=0;c->s=0xFD;c->c=c->z=c->d=c->v=c->n=0;c->i=1;c->halted=false;c->cycles=0;c->pc=read16(c,0xFFFC);}
static void interrupt_enter(TamagoCpu*c,uint16_t vector,bool is_brk,bool set_i){push(c,(uint8_t)(c->pc>>8));push(c,(uint8_t)c->pc);push(c,(uint8_t)(tamago_cpu_status(c)|(is_brk?0x10:0)));if(set_i)c->i=1;c->pc=read16(c,vector);}
bool tamago_cpu_irq(TamagoCpu*c){if(!c||c->halted||c->i)return false;interrupt_enter(c,0xFFFE,false,true);return true;}
void tamago_cpu_nmi(TamagoCpu*c){if(!c||c->halted)return;interrupt_enter(c,0xFFFA,false,false);} /* upstream: NMI leaves I untouched */
static uint16_t addr(TamagoCpu*c,AddrMode m,bool*acc){uint16_t a=0;*acc=false;switch(m){case AM_IMPLIED: return 0;case AM_ACCUMULATOR:*acc=true;return 0;case AM_IMMEDIATE:a=c->pc++;return a;case AM_RELATIVE:{ int8_t off=(int8_t)fetch(c); return (uint16_t)(c->pc+off); }case AM_ZEROPAGE:return fetch(c);case AM_ZEROPAGEX:return (uint8_t)(fetch(c)+c->x);case AM_ZEROPAGEY:return (uint8_t)(fetch(c)+c->y);case AM_ABSOLUTE:return fetch16(c);case AM_ABSOLUTEX:return (uint16_t)(fetch16(c)+c->x);case AM_ABSOLUTEY:return (uint16_t)(fetch16(c)+c->y);case AM_INDIRECT:{uint16_t p=fetch16(c);return (uint16_t)(rd(c,p)|((uint16_t)rd(c,(p&0xFF00)|((p+1)&0xFF))<<8));}case AM_INDIRECTX:{uint8_t p=(uint8_t)(fetch(c)+c->x);return (uint16_t)(rd(c,p)|((uint16_t)rd(c,(uint8_t)(p+1))<<8));}case AM_INDIRECTY:{uint8_t p=fetch(c);return (uint16_t)((rd(c,p)|((uint16_t)rd(c,(uint8_t)(p+1))<<8))+c->y);}default:return 0;}}
static void compare(TamagoCpu*c,uint8_t lhs,uint8_t rhs){uint8_t r=(uint8_t)(lhs-rhs);c->c=lhs>=rhs;nz(c,r);}
static void branch(TamagoCpu*c,bool cond,uint16_t target){if(cond)c->pc=target;}
int tamago_cpu_step(TamagoCpu*c){if(!c||c->halted)return -1;uint8_t code=fetch(c);OpInfo in=op_table[code];if(!in.valid){c->halted=true;return -1;}bool acc=false;uint16_t a=addr(c,in.mode,&acc);uint8_t m=acc?c->a:rd(c,a),r=0; c->cycles-=in.cycles;
 switch(in.op){
 case OP_ADC:{int cin=c->c?1:0;int o=(int)c->a+m+cin;c->v=((~(c->a^m)&(c->a^o)&0x80)!=0);nz(c,(uint8_t)o);
  if(c->d){ /* BCD add: a decimal carry out of the low digit goes into the high digit (the old code lost it: 09+01 gave 00) */
   int al=(c->a&0x0F)+(m&0x0F)+cin;if(al>0x09)al+=0x06;
   int ah=(c->a>>4)+(m>>4)+((al>0x0F)?1:0);if(ah>0x09)ah+=0x06;
   o=(((ah&0x0F)<<4)|(al&0x0F));c->c=ah>0x0F;c->a=(uint8_t)o;break;}
  c->c=(o&~0xFF)!=0;c->a=(uint8_t)o;break;}
 case OP_SBC:{const int bin=c->c?0:1;int o=(int)c->a-m-bin;c->v=(((c->a^m)&(c->a^o)&0x80)!=0);nz(c,(uint8_t)o);c->c=((o&~0xFF)==0);
  if(c->d){ /* BCD subtract with the carry as it was BEFORE this instruction (the old code used the already-updated one) */
   int al=(c->a&0x0F)-(m&0x0F)-bin;int ah=(c->a>>4)-(m>>4);if(al<0){al-=0x06;ah-=1;}if(ah<0)ah-=0x06;
   o=(((ah&0x0F)<<4)|(al&0x0F));}
  c->a=(uint8_t)o;break;}
 case OP_AND:c->a&=m;nz(c,c->a);break;case OP_ORA:c->a|=m;nz(c,c->a);break;case OP_EOR:c->a^=m;nz(c,c->a);break;
 case OP_ASL:r=(uint8_t)(m<<1);c->c=(m>>7)&1;nz(c,r);if(acc)c->a=r;else wr(c,a,r);break;
 case OP_LSR:r=m>>1;c->c=m&1;nz(c,r);if(acc)c->a=r;else wr(c,a,r);break;
 case OP_ROL:r=(uint8_t)((m<<1)|(c->c?1:0));c->c=(m>>7)&1;nz(c,r);if(acc)c->a=r;else wr(c,a,r);break;
 case OP_ROR:r=(uint8_t)((m>>1)|(c->c?0x80:0));c->c=m&1;nz(c,r);if(acc)c->a=r;else wr(c,a,r);break;
 case OP_BIT:c->z=(c->a&m)==0;c->n=(m>>7)&1;c->v=(m>>6)&1;break;
 case OP_CMP:compare(c,c->a,m);break;case OP_CPX:compare(c,c->x,m);break;case OP_CPY:compare(c,c->y,m);break;
 case OP_DEC:r=(uint8_t)(m-1);wr(c,a,r);nz(c,r);break;case OP_INC:r=(uint8_t)(m+1);wr(c,a,r);nz(c,r);break;
 case OP_DEX:c->x--;nz(c,c->x);break;case OP_DEY:c->y--;nz(c,c->y);break;case OP_INX:c->x++;nz(c,c->x);break;case OP_INY:c->y++;nz(c,c->y);break;
 case OP_LDA:c->a=m;nz(c,m);break;case OP_LDX:c->x=m;nz(c,m);break;case OP_LDY:c->y=m;nz(c,m);break;
 case OP_STA:wr(c,a,c->a);break;case OP_STX:wr(c,a,c->x);break;case OP_STY:wr(c,a,c->y);break;
 case OP_TAX:c->x=c->a;nz(c,c->x);break;case OP_TAY:c->y=c->a;nz(c,c->y);break;case OP_TSX:c->x=c->s;nz(c,c->x);break;case OP_TXA:c->a=c->x;nz(c,c->a);break;case OP_TYA:c->a=c->y;nz(c,c->a);break;case OP_TXS:c->s=c->x;break;
 case OP_CLC:c->c=0;break;case OP_SEC:c->c=1;break;case OP_CLI:c->i=0;break;case OP_SEI:c->i=1;break;case OP_CLD:c->d=0;break;case OP_SED:c->d=1;break;case OP_CLV:c->v=0;break;case OP_NOP:break;
 case OP_BCC:branch(c,!c->c,a);break;case OP_BCS:branch(c,c->c,a);break;case OP_BEQ:branch(c,c->z,a);break;case OP_BNE:branch(c,!c->z,a);break;case OP_BMI:branch(c,c->n,a);break;case OP_BPL:branch(c,!c->n,a);break;case OP_BVC:branch(c,!c->v,a);break;case OP_BVS:branch(c,c->v,a);break;
 case OP_JMP:c->pc=a;break;case OP_JSR:push(c,(uint8_t)((c->pc-1)>>8));push(c,(uint8_t)(c->pc-1));c->pc=a;break;case OP_RTS:{uint8_t lo=pop(c);uint8_t hi=pop(c);c->pc=(uint16_t)((lo|((uint16_t)hi<<8))+1);}break;
 case OP_PHA:push(c,c->a);break;case OP_PHP:push(c,(uint8_t)(tamago_cpu_status(c)|0x10));break;case OP_PLA:c->a=pop(c);nz(c,c->a);break;case OP_PLP:tamago_cpu_set_status(c,pop(c));break;
 case OP_RTI:{tamago_cpu_set_status(c,pop(c));uint8_t lo=pop(c);uint8_t hi=pop(c);c->pc=(uint16_t)(lo|((uint16_t)hi<<8));}break;
 case OP_BRK:c->pc++;push(c,(uint8_t)(c->pc>>8));push(c,(uint8_t)c->pc);push(c,(uint8_t)(tamago_cpu_status(c)|0x10));c->i=1;c->pc=read16(c,0xFFFE);break;
 default:c->halted=true;return -1;
 }
 return 0;
}
