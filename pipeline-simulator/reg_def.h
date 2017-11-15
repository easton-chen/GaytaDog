typedef unsigned long long REG;

struct IFID{
    unsigned int inst;
    long long PC;
    long long val_P;
}IF_ID,IF_ID_old;

void print_IFID()
{
    printf("inst = %08x\n", IF_ID_old.inst);
    printf("PC = %llx\n", IF_ID_old.PC);
}

struct IDEX{
    int Rd,Rt;
    long long PC;
    long long val_P;
    long long Imm;
    int Reg_dst;
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
    printf("Rd = %08x\n", ID_EX_old.Rd);
    printf("Rt = %08x\n", ID_EX_old.Rt);
    printf("PC = %llx\n", ID_EX_old.PC);
    printf("Imm = %llx\n", ID_EX_old.Imm);
    printf("Reg_dst = %x\n", ID_EX_old.Reg_dst);
    printf("Reg_Rs = %llx\n", ID_EX_old.Reg_Rs);
    printf("Reg_Rt = %llx\n", ID_EX_old.Reg_Rt);
    printf("Ctrl_EX_ALUSrc = %x\n", ID_EX_old.Ctrl_EX_ALUSrc);
    printf("Ctrl_EX_ALUOp = %d\n", ID_EX_old.Ctrl_EX_ALUOp);
    printf("Ctrl_EX_RegDst = %x\n", ID_EX_old.Ctrl_EX_RegDst);
    printf("Ctrl_M_Branch = %x\n", ID_EX_old.Ctrl_M_Branch);
    printf("Ctrl_M_MemWrite = %x\n", ID_EX_old.Ctrl_M_MemWrite);
    printf("Ctrl_M_MemRead = %x\n", ID_EX_old.Ctrl_M_MemRead);
    printf("Ctrl_WB_RegWrite = %x\n", ID_EX_old.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n", ID_EX_old.Ctrl_WB_MemtoReg);
}

struct EXMEM{
    long long PC;
    long long val_P;
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
    printf("PC = %llx\n",EX_MEM_old.PC);
    printf("Reg_dst = %x\n",EX_MEM_old.Reg_dst);
    printf("ALU_out = %llx\n",EX_MEM_old.ALU_out);
    printf("Zero = %x\n",EX_MEM_old.Zero);
    printf("Reg_Rt = %llx\n",EX_MEM_old.Reg_Rt);
    printf("Ctrl_EX_ALUOp = %d\n",EX_MEM_old.Ctrl_EX_ALUOp);
    printf("Ctrl_M_Branch = %x\n",EX_MEM_old.Ctrl_M_Branch);
    printf("Ctrl_M_MemWrite = %x\n",EX_MEM_old.Ctrl_M_MemWrite);
    printf("Ctrl_M_MemRead = %x\n",EX_MEM_old.Ctrl_M_MemRead);
    printf("Ctrl_WB_RegWrite = %x\n",EX_MEM_old.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n",EX_MEM_old.Ctrl_WB_MemtoReg);
}

struct MEMWB{
    long long PC;
    long long val_P;
    unsigned int Mem_read;
    REG ALU_out;
    int Reg_dst;

    char Ctrl_WB_RegWrite;
    char Ctrl_WB_MemtoReg;

}MEM_WB,MEM_WB_old;

void print_MEMWB()
{
    printf("PC = %llx\n",MEM_WB_old.PC);
    printf("Mem_read = %x\n",MEM_WB_old.Mem_read);
    printf("ALU_out = %llx\n",MEM_WB_old.ALU_out);
    printf("Reg_dst = %x\n",MEM_WB_old.Reg_dst);
    printf("Ctrl_WB_RegWrite = %x\n",MEM_WB_old.Ctrl_WB_RegWrite);
    printf("Ctrl_WB_MemtoReg = %x\n",MEM_WB_old.Ctrl_WB_MemtoReg);
}
