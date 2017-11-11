#ifndef REG_DEF_H
#define REG_DEF_H
typedef unsigned long long REG;

struct IFID{
    unsigned int inst;
    long long PC;
}IF_ID,IF_ID_old;

void print_IFID()
{
    printf("inst = %08x\n", IF_ID.inst);
    printf("PC = %llx\n", IF_ID.PC);
}

struct IDEX{
    int Rd,Rt;
    long long PC;
    long long Imm;
    REG Reg_Rs,Reg_Rt;

    char Ctrl_EX_ALUSrc;
    char Ctrl_EX_ALUOp;
    char Ctrl_EX_RegDst;

    char Ctrl_M_Branch;
    char Ctrl_M_MemWrite;
    char Ctrl_M_MemRead;

    char Ctrl_WB_RegWrite;
    char Ctrl_WB_MemtoReg;

}ID_EX,ID_EX_old;

void print_IDEX()
{
    printf("Rd = %08x\n", ID_EX.Rd);
    printf("Rt = %08x\n", ID_EX.Rt);
    printf("PC = %llx\n", ID_EX.PC);
    printf("Imm = %llx\n", ID_EX.Imm);
    printf("Reg_Rs = %llx\n", ID_EX.Reg_Rs);
    printf("Reg_Rt = %llx\n", ID_EX.Reg_Rt);
    printf("Ctrl_EX_ALUSrc = %x\n", ID_EX.Ctrl_EX_ALUSrc);
    printf("Ctrl_EX_ALUOp = %x\n", ID_EX.Ctrl_EX_ALUOp);
    printf("Ctrl_EX_RegDst = %x\n", ID_EX.Ctrl_EX_RegDst);
    printf("Ctrl_M_Branch = %x\n", ID_EX.Ctrl_M_Branch);
    printf("Ctrl_M_MemWrite = %x\n", ID_EX.Ctrl_M_MemWrite);
    printf("Ctrl_M_MemRead = %x\n", ID_EX.Ctrl_M_MemRead);
    printf("Ctrl_WB_RegWrite = %x\n", ID_EX.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n", ID_EX.Ctrl_WB_MemtoReg);
}

struct EXMEM{
    long long PC;
    int Reg_dst;
    REG ALU_out;
    int Zero;
    REG Reg_Rt;

    char Ctrl_EX_ALUOp;

    char Ctrl_M_Branch;
    char Ctrl_M_MemWrite;
    char Ctrl_M_MemRead;

    char Ctrl_WB_RegWrite;
    char Ctrl_WB_MemtoReg;

}EX_MEM,EX_MEM_old;

void print_EXMEM()
{
    printf("PC = %llx\n",EX_MEM.PC);
    printf("Reg_dst = %x\n",EX_MEM.Reg_dst);
    printf("ALU_out = %llx\n",EX_MEM.ALU_out);
    printf("Zero = %x\n",EX_MEM.Zero);
    printf("Reg_Rt = %llx\n",EX_MEM.Reg_Rt);
    printf("Ctrl_EX_ALUOp = %x\n",EX_MEM.Ctrl_EX_ALUOp);
    printf("Ctrl_M_Branch = %x\n",EX_MEM.Ctrl_M_Branch);
    printf("Ctrl_M_MemWrite = %x\n",EX_MEM.Ctrl_M_MemWrite);
    printf("Ctrl_M_MemRead = %x\n",EX_MEM.Ctrl_M_MemRead);
    printf("Ctrl_WB_RegWrite = %x\n",EX_MEM.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n",EX_MEM.Ctrl_WB_MemtoReg);
}

struct MEMWB{
    unsigned int Mem_read;
    REG ALU_out;
    int Reg_dst;

    char Ctrl_WB_RegWrite;
    char Ctrl_WB_MemtoReg;

}MEM_WB,MEM_WB_old;

void print_MEMWB()
{
    printf("Mem_read = %x\n",MEM_WB.Mem_read);
    printf("ALU_out = %llx\n",MEM_WB.ALU_out);
    printf("Reg_dst = %x\n",MEM_WB.Reg_dst);
    printf("Ctrl_WB_RegWrite = %x\n",MEM_WB.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n",MEM_WB.Ctrl_WB_MemtoReg);
}
#endif // REG_DEF_H
