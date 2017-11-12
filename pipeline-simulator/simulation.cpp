#include "simulation.h"
using namespace std;

extern FILE *file;

extern bool read_elf();
extern unsigned long long cadr; //代码段在解释文件中的偏移地址
extern unsigned long long csize; //代码段的长度
extern unsigned long long vcadr; //代码段在内存中的虚拟地址

extern unsigned long long dadr; //数据段在解释文件中的偏移地址
extern unsigned long long dsize; //数据段的长度
extern unsigned long long bsize; //.bss和.sbss段的长度
extern unsigned long long vdadr; //数据段在内存中的虚拟地址
extern unsigned long long gp; //全局数据段在内存的地址

extern unsigned long long madr; //main函数在内存中地址
extern unsigned long long msize; //main函数的长度

extern unsigned long long entry; //程序的入口地址
extern unsigned long long endPC; //程序结束时的PC

//程序运行的指令数
long long inst_num=0;
long long cycle_num=0;

//mul-div flag
bool mul_flag = false;
bool div_flag = false;
bool rem_flag = false;

#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

//将代码段和数据段加载到内存中
void load_memory()
{
    memset(memory,0,sizeof(memory));

    //用cadr和csize映射.text段
    fseek(file,cadr,SEEK_SET);
    fread(&memory[vcadr],1,csize,file);

#ifdef DEBUG
    {
        dbg_printf( "cadr = %llx\n", cadr);
        dbg_printf( "csize = %llx\n", csize);
        dbg_printf( "vcadr = %llx\n", vcadr);
        for(int i = 0; i < csize; i++)
        {
            dbg_printf( "%02x",memory[i+vcadr]);
            if((i+1)%4 == 0) dbg_printf( " ");
            if((i+1)%16 == 0) dbg_printf( "\n");
        }
        dbg_printf( "\n.text finished!\n\n\n");
    }
#endif


    //用dadr和dsize-bsize映射.data段
    fseek(file,dadr,SEEK_SET);
    fread(&memory[vdadr],1,dsize-bsize,file);

#ifdef DEBUG
    {
        dbg_printf( "dadr = %llx\n",dadr);
        dbg_printf( "dsize = %llx\n",dsize);
        dbg_printf( "vdadr = %llx\n",vdadr);
        for(int i = 0; i < dsize; i++)
        {
            dbg_printf( "%02x",memory[i+vdadr]);
            if((i+1)%4 == 0) dbg_printf( " ");
            if((i+1)%16==0) dbg_printf( "\n");
        }
        dbg_printf( "\n.data finished!\n\n\n");
    }
#endif

    entry = vcadr;
    endPC=madr+msize-4;

#ifdef DEBUG
    {
        dbg_printf( "entry = %llx\n",entry);
        dbg_printf( "gp = %llx\n",gp);
        dbg_printf( "madr = %llx\n",madr);
        dbg_printf( "endPC = %llx\n",endPC);
    }
#endif

    fclose(file);
}

//初始化全局变量，用于图形界面中的load按钮
void load()
{
    //解析elf文件
    if(!read_elf())
    {
        return;
    }

    //加载到内存
    load_memory();

    //设置入口地址为主函数地址
    PC=madr;

    //设置sp寄存器/栈基址
    reg[2]=MAX/2;

    //设置全局数据段地址寄存器
    reg[3]=gp;
}

int main()
{
    inst_num=0;
    cycle_num=0;
    mul_flag=false;
    div_flag=false;
    rem_flag=false;
    load();
    simulate(0);
    cout << "simulation over!" << endl;
    cout << "instruction num:" << inst_num << endl;
    cout << "cycle num:" << cycle_num << endl;
    cout << "CPI:" << (double)cycle_num/(double)inst_num << endl;

}

//simulation.cpp的主函数,if_debug表示是否为单步调试模式
void simulate(int if_debug)
{
    while(PC!=endPC)
    {
#ifdef DEBUG
        {
            dbg_printf( "=======================================================\n");
            dbg_printf( "instruction num: %d\n",inst_num);
        }
#endif

        IF();
        ID();
        EX();
        MEM();
        WB();

        //更新中间寄存器
        //IF_ID=IF_ID_old;
        //ID_EX=ID_EX_old;
        //EX_MEM=EX_MEM_old;
        //MEM_WB=MEM_WB_old;
        update_latch();
        reg[0]=0;//一直为零

        //update cycle number
        cycle_num++;
#ifdef DEBUG
        {
        print_REG();
        dbg_printf( "=======================================================\n\n\n");
        }
#endif

        if(if_debug==1) break;
    }
    if(PC==endPC)
        cout<<"Simulation finished"<<endl;
}

//取指
void IF()
{
#ifdef DEBUG
    {
        dbg_printf( "-------------IF--------------\n");
        dbg_printf( "PC:%llx\n",PC);
    }
#endif

    //write IF_ID_old
    //IF_ID_old.inst=memory[PC];
    memcpy(&IF_ID_old.inst,memory+PC,4);
    Inst=IF_ID_old.inst;
    IF_ID_old.PC=PC;
    IF_ID_old.val_P=PC+4;

#ifdef DEBUG
    {
        dbg_printf( "instruction:%08x\n",IF_ID_old.inst);
        dbg_printf( "IF finished\n");
        print_IFID();
    }
#endif
    unsigned int OP=getbit(Inst,25,31);
    long long Imm;
    unsigned int rs=0;
    unsigned int fuc3=0;
    if(OP==OP_JALR)//0x67
    {
        rs=getbit(Inst,12,16);
        fuc3=getbit(Inst,17,19);
        if(fuc3==0x00)
        {
            Imm=ext_signed(getbit(Inst,0,11),1,12);
            PC=reg[rs]+Imm;//PredictPC
        }
        else dbg_printf("Invalid instruction\n");
    }
    else if(OP==OP_BEQ)//0x63
    {
        Imm=ext_signed(((getbit(Inst,0,0)<<12) + (getbit(Inst,24,24)<<11) + (getbit(Inst,1,6)<<5) + (getbit(Inst,20,23)<<1)),1,13);
        fuc3=getbit(Inst,17,19);

        switch(fuc3)
        {
            case 0:
                PC=PC+Imm;//if(R[rs1] == R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 1:
                PC=PC+Imm;//if(R[rs1] != R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 4:
                PC=PC+Imm;//if(R[rs1] < R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 5:
                PC=PC+Imm;//if(R[rs1] >= R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_JAL)//0x6f
    {
        Imm=ext_signed(((getbit(Inst,0,0)<<20) + (getbit(Inst,12,19)<<12) + (getbit(Inst,11,11)<<11) + (getbit(Inst,1,10)<<1)),1,21);
        PC=PC+Imm;
    }
    else
    PC=PC+4;
}

//译码
void ID()
{
#ifdef DEBUG
    {
        dbg_printf( "-------------ID--------------\n");
    }
#endif

    //Read IF_ID
    unsigned int inst=IF_ID.inst;

    int EXTop=0;
    unsigned int Imm_length=0;

    char RegDst,ALUop,ALUSrc;
    char Branch,MemRead,MemWrite;
    char RegWrite,MemtoReg;

    unsigned int OP=getbit(inst,25,31);

#ifdef DEBUG
    {
        dbg_printf( "OP:%02x\n",OP);
    }
#endif

    unsigned int rs=0;
    unsigned int rt=0;
    unsigned int rd=0;
    unsigned int fuc3=0;
    unsigned int fuc7=0;
    unsigned int Imm=0;

    if(OP==OP_R)//0x33
    {
        fuc7=getbit(inst,0,6);
        rt=getbit(inst,7,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);

        Imm_length=0;
        EXTop=0;
        RegDst=1;
        ALUop=0;
        ALUSrc=0;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;

        switch(fuc3){
            case 0:
                switch(fuc7)
                {
                    case 0x00:
                        ALUop=1;// R[rd] ← R[rs1] + R[rs2]
                        break;
                    case 0x01:
                        ALUop=2;//R[rd] ← (R[rs1] * R[rs2])[63:0]
                        break;
                    case 0x20:
                        ALUop=3;//R[rd] ← R[rs1] - R[rs2]
                        break;
                    default:
                        dbg_printf("Invalid instruction\n");
                        break;
                }
                break;
            case 1:
                if(fuc7==0x00) ALUop=4;//R[rd] ← R[rs1] << R[rs2]
                else if(fuc7==0x01) ALUop=5;//R[rd] ← (R[rs1] * R[rs2])[127:64]
                else dbg_printf("Invalid instruction\n");
                break;
            case 2:
                if(fuc7==0x00) ALUop=6;//R[rd] ← (R[rs1] < R[rs2]) ? 1 : 0
                else dbg_printf("Invalid instruction\n");
                break;
            case 4:
                if(fuc7 == 0x00) ALUop=7;//R[rd] ← R[rs1] ^ R[rs2]
                else if(fuc7==0x01) ALUop=8;//R[rd] ← R[rs1] / R[rs2]
                else dbg_printf("Invalid instruction\n");
                break;
            case 5:
                if(fuc7 == 0x00) ALUop=9;//srl:R[rd] ← R[rs1] >> R[rs2]
                else if(fuc7==0x20) ALUop=38;//sra:R[rd] ← R[rs1] >> R[rs2]
                else dbg_printf("Invalid instruction\n");
                break;
            case 6:
                if(fuc7 == 0x00) ALUop=10;//R[rd] ← R[rs1] | R[rs2]
                else if(fuc7==0x01) ALUop=11;//R[rd] ← R[rs1] % R[rs2]
                else dbg_printf("Invalid instruction\n");
                break;
            case 7:
                if(fuc7==0x00) ALUop=12;//R[rd] ← R[rs1] & R[rs2]
                else dbg_printf("Invalid instruction\n");
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_I)//0x13
    {
        Imm=getbit(inst,0,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);
        fuc7=getbit(inst,0,6);
        Imm_length=12;
        EXTop=0;
        RegDst=1;
        ALUop=0;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;

        switch(fuc3)
        {
            case 0:
                ALUop=17;//R[rd] ← R[rs1] + imm
                EXTop=1;
                break;
            case 1:
                if(fuc7==0x00) ALUop=18;//R[rd] ← R[rs1] << imm
                else dbg_printf("Invalid instruction\n");
                break;
            case 2:
                ALUop=19;//R[rd] ← (R[rs1] < imm) ? 1 : 0
                break;
            case 4:
                ALUop=20;//R[rd] ← R[rs1] ^ imm
                break;
            case 5:
                if(fuc7 == 0x00) ALUop=21;//srli:R[rd] ← R[rs1] >> imm
                else if(fuc7==0x20) ALUop=39;//srai:R[rd] ← R[rs1] >> imm
                else dbg_printf("Invalid instruction\n");
                break;
            case 6:
                ALUop=22;//R[rd] ← R[rs1] | imm
                break;
            case 7:
                ALUop=23;//R[rd] ← R[rs1] & imm
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_SW)//0x23
    {
        Imm=(getbit(inst,0,6)<<5) + getbit(inst,20,24);
        rt=getbit(inst,7,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        Imm_length=12;
        EXTop=1;
        RegDst=0;
        ALUop=0;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=1;
        RegWrite=0;
        MemtoReg=0;

        switch(fuc3)
        {
            case 0:
                ALUop=27;//Mem(R[rs1] + offset) ← R[rs2][7:0]
                break;
            case 1:
                ALUop=28;//Mem(R[rs1] + offset) ← R[rs2][15:0]
                break;
            case 2:
                ALUop=29;//Mem(R[rs1] + offset) ← R[rs2][31:0]
                break;
            case 3:
                ALUop=30;//Mem(R[rs1] + offset) ← R[rs2][63:0]
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_LW)//0x03
    //LW指令类从内存中读32位数，符号扩展到64位存储在rd中
    {
        Imm=getbit(inst,0,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);
        Imm_length=12;
        EXTop=1;
        RegDst=1;
        ALUop=0;
        ALUSrc=1;
        Branch=0;
        MemRead=1;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=1;

        switch(fuc3)
        {
            case 0:
                ALUop=13;//R[rd] ← SignExt(Mem(R[rs1] + offset, byte))
                break;
            case 1:
                ALUop=14;//R[rd] ← SignExt(Mem(R[rs1] + offset, half))
                break;
            case 2:
                ALUop=15;//R[rd] ← Mem(R[rs1] + offset, word)
                break;
            case 3:
                ALUop=16;//R[rd] ← Mem(R[rs1] + offset, doubleword)
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_BEQ)//0x63
    {
        Imm=(getbit(inst,0,0)<<12) + (getbit(inst,24,24)<<11) + (getbit(inst,1,6)<<5) + (getbit(inst,20,23)<<1);
        rt=getbit(inst,7,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        Imm_length=13;
        EXTop=1;
        RegDst=0;
        ALUop=0;
        ALUSrc=0;
        Branch=1;
        MemRead=0;
        MemWrite=0;
        RegWrite=0;
        MemtoReg=0;

        switch(fuc3)
        {
            case 0:
                ALUop=31;//if(R[rs1] == R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 1:
                ALUop=32;//if(R[rs1] != R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 4:
                ALUop=33;//if(R[rs1] < R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            case 5:
                ALUop=34;//if(R[rs1] >= R[rs2]) PC ← PC + {offset, 1b'0}
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_JAL)//0x6f
    {
        Imm=(getbit(inst,0,0)<<20) + (getbit(inst,12,19)<<12) + (getbit(inst,11,11)<<11) + (getbit(inst,1,10)<<1);
        rd=getbit(inst,20,24);
        Imm_length=21;
        EXTop=1;
        RegDst=1;
        ALUop=37;
        ALUSrc=1;
        Branch=1;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;
    }
    else if(OP==OP_IW)//0x1B
    {
        Imm=getbit(inst,0,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);
        fuc7=getbit(inst,0,6);
        unsigned int imm5=getbit(inst,6,6);//imm[5]

        switch(fuc3)
        {
            case 0:
                Imm_length=12;
                EXTop=1;
                RegDst=1;
                ALUop=24;
                ALUSrc=1;
                Branch=0;
                MemRead=0;
                MemWrite=0;
                RegWrite=1;
                MemtoReg=0;
                break;
            case 1://slliw
                if(imm5!=0)
                {
                    dbg_printf("Invalid instruction\n");
                    break;
                }
                else
                {
                    Imm_length=12;
                    EXTop=1;
                    RegDst=1;
                    ALUop=41;
                    ALUSrc=1;
                    Branch=0;
                    MemRead=0;
                    MemWrite=0;
                    RegWrite=1;
                    MemtoReg=0;
                    break;
                }
                break;
            case 5:
                if(imm5!=0)
                {
                    dbg_printf("Invalid instruction\n");
                    break;
                }
                else if(fuc7==0x00)//srliw
                {
                    Imm_length=12;
                    EXTop=1;
                    RegDst=1;
                    ALUop=42;
                    ALUSrc=1;
                    Branch=0;
                    MemRead=0;
                    MemWrite=0;
                    RegWrite=1;
                    MemtoReg=0;
                    break;
                }
                else if(fuc7==32)//sraiw
                {
                    Imm=getbit(inst,7,11);
                    Imm_length=5;
                    EXTop=1;
                    RegDst=1;
                    ALUop=43;
                    ALUSrc=1;
                    Branch=0;
                    MemRead=0;
                    MemWrite=0;
                    RegWrite=1;
                    MemtoReg=0;
                    break;
                }
                else
                {
                    dbg_printf("Invalid instruction\n");
                    break;
                }
                break;
            default:
                dbg_printf("Invalid instruction\n");
                break;
        }
    }
    else if(OP==OP_JALR)//0x67
    {
        Imm=getbit(inst,0,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);
        if(fuc3==0x00)
        {
            Imm_length=12;
            EXTop=1;
            RegDst=1;
            ALUop=25;
            ALUSrc=1;
            Branch=1;
            MemRead=0;
            MemWrite=0;
            RegWrite=1;
            MemtoReg=0;
        }
        else dbg_printf("Invalid instruction\n");
    }
    else if(OP==OP_SCALL)//0x73
    {
        Imm=getbit(inst,0,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);
        fuc7=getbit(inst,0,6);
        if(fuc3==0x0&&fuc7==0x0)
        {
            Imm_length=12;
            EXTop=0;
            RegDst=0;
            ALUop=26;
            ALUSrc=0;
            Branch=0;
            MemRead=0;
            MemWrite=0;
            RegWrite=0;
            MemtoReg=0;
        }
        else dbg_printf("Invalid instruction\n");
    }
    else if(OP==OP_AUIPC)//0x17
    {
        Imm=getbit(inst,0,19)<<12;
        rd=getbit(inst,20,24);
        Imm_length=32;
        EXTop=1;
        RegDst=1;
        ALUop=35;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;
    }
    else if(OP==OP_LUI)//0x37
    {
        Imm=getbit(inst,0,19)<<12;
        rd=getbit(inst,20,24);
        Imm_length=32;
        EXTop=1;
        RegDst=1;
        ALUop=36;
        ALUSrc=1;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;
    }
    else if(OP==OP_MUL_EXTEND)//RV64M Standard Extension
    {
        fuc7=getbit(inst,0,6);
        rt=getbit(inst,7,11);
        rs=getbit(inst,12,16);
        fuc3=getbit(inst,17,19);
        rd=getbit(inst,20,24);

        Imm_length=0;
        EXTop=0;
        RegDst=1;
        ALUop=0;
        ALUSrc=0;
        Branch=0;
        MemRead=0;
        MemWrite=0;
        RegWrite=1;
        MemtoReg=0;

        if(fuc7==0&&fuc3==0)//addw
        {
            ALUop=44;
        }
        else if(fuc7==1)
            switch(fuc3)
            {
                case 0:
                    ALUop=40;
                    break;
                default:
                    dbg_printf("Invalid instruction\n");
                    break;
            }
    }
    //check for the data risk
    if(ID_EX.Ctrl_WB_RegWrite==1 && (rs==ID_EX.Reg_dst||rt==ID_EX.Reg_dst))
    {
        stall_flag[0]=1;
        stall_flag[1]=1;
        bubble_flag[2]=1;
    }
    else if(EX_MEM.Ctrl_WB_RegWrite==1 && (rs==EX_MEM.Reg_dst||rt==EX_MEM.Reg_dst))
    {
        stall_flag[0]=1;
        stall_flag[1]=1;
        bubble_flag[2]=1;
    }
    else if(MEM_WB.Ctrl_WB_RegWrite==1 && (rs==MEM_WB.Reg_dst||rt==MEM_WB.Reg_dst))
    {
        stall_flag[0]=1;
        stall_flag[1]=1;
        bubble_flag[2]=1;
    }
    //choose reg dst address                                       
    int Reg_Dst;
    if(RegDst)
    {
        Reg_Dst=rd;
    }
    else
    {
        Reg_Dst=rt;
    }


    //write ID_EX_old
    ID_EX_old.Rd=rd;
    ID_EX_old.Rt=rt;
    ID_EX_old.Reg_dst=Reg_Dst;
    ID_EX_old.Reg_Rs=reg[rs];
    ID_EX_old.Reg_Rt=reg[rt];

    ID_EX_old.PC=IF_ID.PC;
    dbg_printf( "EXTop = %d\n",EXTop);
    dbg_printf( "Imm_length = %d\n",Imm_length);
    ID_EX_old.Imm=ext_signed(Imm,EXTop,Imm_length);

    ID_EX_old.Ctrl_EX_ALUSrc=ALUSrc;
    ID_EX_old.Ctrl_EX_ALUOp=ALUop;
    ID_EX_old.Ctrl_EX_RegDst=RegDst;

    ID_EX_old.Ctrl_M_Branch=Branch;
    ID_EX_old.Ctrl_M_MemWrite=MemWrite;
    ID_EX_old.Ctrl_M_MemRead=MemRead;

    ID_EX_old.Ctrl_WB_RegWrite=RegWrite;
    ID_EX_old.Ctrl_WB_MemtoReg=MemtoReg;

#ifdef DEBUG
{
    dbg_printf("Inst: \n");
    switch (ALUop)
    {
        case 1: dbg_printf("R[rd] ← R[rs1] + R[rs2]\n"); break;
        case 2: dbg_printf("R[rd] ← (R[rs1] * R[rs2])[63:0]\n");break;
        case 3: dbg_printf("R[rd] ← R[rs1] - R[rs2]\n");break;
        case 4: dbg_printf("R[rd] ← R[rs1] << R[rs2]\n");break;
        case 5: dbg_printf("R[rd] ← (R[rs1] * R[rs2])[127:64]\n");break;
        case 6: dbg_printf("R[rd] ← (R[rs1] < R[rs2]) ? 1 : 0\n");break;
        case 7: dbg_printf("R[rd] ← R[rs1] ^ R[rs2]\n");break;
        case 8: dbg_printf("R[rd] ← R[rs1] / R[rs2]\n");break;
        case 9: dbg_printf("srl:R[rd] ← R[rs1] >> R[rs2]\n");break;
        case 10: dbg_printf("R[rd] ← R[rs1] | R[rs2]\n");break;
        case 11: dbg_printf("R[rd] ← (R[rs1] %% R[rs2]\n");break;
        case 12: dbg_printf("R[rd] ← R[rs1] & R[rs2]\n");break;
        case 13: dbg_printf("R[rd] ← SignExt(Mem(R[rs1] + offset, byte))\n");break;
        case 14: dbg_printf("R[rd] ← SignExt(Mem(R[rs1] + offset, half))\n");break;
        case 15: dbg_printf("R[rd] ← Mem(R[rs1] + offset, word)\n");break;
        case 16: dbg_printf("R[rd] ← Mem(R[rs1] + offset, doubleword)\n");break;
        case 17: dbg_printf("R[rd] ← R[rs1] + imm\n");break;
        case 18: dbg_printf("R[rd] ← R[rs1] << imm\n");break;
        case 19: dbg_printf("R[rd] ← (R[rs1] < imm) ? 1 : 0\n");break;
        case 20: dbg_printf("R[rd] ← R[rs1] ^ imm\n");break;
        case 21: dbg_printf("srli:R[rd] ← R[rs1] >> imm\n");break;
        case 22: dbg_printf("R[rd] ← R[rs1] | imm\n");break;
        case 23: dbg_printf("R[rd] ← R[rs1] & imm\n");break;
        case 24: dbg_printf("R[rd] ← SignExt(R[rs1](31:0) + imm)\n");break;
        case 25: dbg_printf("R[rd] ← PC + 4\nPC ← R[rs1] + {imm, 1b'0}\n");break;
        case 26: dbg_printf("(Transfers control to operating system)\n");break;
        case 27: dbg_printf("Mem(R[rs1] + offset) ← R[rs2][7:0]\n");break;
        case 28: dbg_printf("Mem(R[rs1] + offset) ← R[rs2][15:0]\n");break;
        case 29: dbg_printf("Mem(R[rs1] + offset) ← R[rs2][31:0]\n");break;
        case 30: dbg_printf("Mem(R[rs1] + offset) ← R[rs2][63:0]\n");break;
        case 31: dbg_printf("if(R[rs1] == R[rs2])\n");break;
        case 32: dbg_printf("if(R[rs1] != R[rs2])\n");break;
        case 33: dbg_printf("if(R[rs1] < R[rs2])\n");break;
        case 34: dbg_printf("if(R[rs1] >= R[rs2])\n");break;
        case 35: dbg_printf("R[rd] ← PC + {offset, 12'b0}\n");break;
        case 36: dbg_printf("R[rd] ← {offset, 12'b0}\n");break;
        case 37: dbg_printf("R[rd] ← PC + 4 PC ← PC + {imm, 1b'0}\n");break;
        case 38: dbg_printf("sra:R[rd] ← R[rs1] >> R[rs2]\n");break;
        case 39: dbg_printf("srai:R[rd] ← R[rs1] >> imm\n");break;
        case 40: dbg_printf("mulw:R[rd] ← R[rs1]*R[rs2][31:0]\n");break;
        case 41:dbg_printf("slliw:R[rd] ← R[rs1] << imm [31:0]\n");break;
        case 42:dbg_printf("srliw:R[rd] ← R[rs1] >> imm\n[31:0]\n");break;
        case 43:dbg_printf("sraiw:R[rd] ← R[rs1] >> imm[31:0]\n");break;
        case 44:dbg_printf("addw\n");break;
        default: dbg_printf("Invalid instruction\n");break;

    }

    dbg_printf("ID finished\n");
    print_IDEX();
}
#endif
}

//执行
void EX()
{
    dbg_printf("-------------EX--------------\n");
    //read ID_EX
    unsigned int rd=ID_EX.Rd;
    unsigned int rt=ID_EX.Rt;
    long long Imm=ID_EX.Imm;
    long long temp_PC=ID_EX.PC;

    REG Rs=ID_EX.Reg_Rs;
    REG Rt=ID_EX.Reg_Rt;
    REG ALUout;

    char ALUSrc=ID_EX.Ctrl_EX_ALUSrc;
    char ALUop=ID_EX.Ctrl_EX_ALUOp;
    char RegDst=ID_EX.Ctrl_EX_RegDst;

    int Zero;
    
    switch(ALUop){
        case 1:
            ALUout=Rs+Rt;
            break;
        case 2:
            ALUout=Rs*Rt;
            if(!mul_flag) cycle_num+=1;
            break;
        case 3:
            ALUout=Rs-Rt;
            break;
        case 4:
            ALUout=Rs<<Rt;
            break;
        case 5:
            ALUout=mulh(Rs,Rt);
            if(!mul_flag) cycle_num+=1;
            break;
        case 6:
            if(Rs<Rt) ALUout=1;
            else ALUout=0;
            break;
        case 7:
            ALUout=Rs^Rt;
            break;
        case 8:
            ALUout=Rs/Rt;
            if(!rem_flag) cycle_num+=39;
            break;
        case 9:
            ALUout=(unsigned long long)Rs>>Rt;
            break;
        case 10:
            ALUout=Rs|Rt;
            break;
        case 11:
            ALUout=Rs%Rt;
            if(!div_flag) cycle_num+=39;
            break;
        case 12:
            ALUout=Rs&Rt;
            break;
        case 13:
            ALUout=Rs+Imm;  
            break;
        case 14:
            ALUout=Rs+Imm;
            break;
        case 15:
            ALUout=Rs+Imm;
            break;
        case 16:
            ALUout=Rs+Imm;
            break;
        case 17:
            ALUout=Rs+Imm;
            break;
        case 18:
            ALUout=Rs<<Imm;
            break;
        case 19:
            if(Rs<Imm)ALUout=1;
            else ALUout=0;
            break;
        case 20:
            ALUout=Rs^Imm;
            break;
        case 21:
            ALUout=(unsigned long long)Rs>>Imm;
            break;
        case 22:
            ALUout=Rs|Imm;
            break;
        case 23:
            ALUout=Rs&Imm;
            break;
        case 24:
            ALUout=ext_signed((Rs + Imm)&0xffffffff,1,32);
            break;
        case 25:
            ALUout=temp_PC+4;
            //PC=Rs+Imm;
            break;
        case 26:
            dbg_printf("System call\n");
            break;
        case 27:
            ALUout=Rs+Imm;
            break;
        case 28:
            ALUout=Rs+Imm;
            break;
        case 29:
            ALUout=Rs+Imm;
            break;
        case 30:
            ALUout=Rs+Imm;
            break;
        case 31:
            //if(Rs==Rt) PC=temp_PC+Imm;
            if((long long)Rs!=(long long)Rt) 
            {
                bubble_flag[1]=bubble_flag[2]=1;
                PC=ID_EX.val_P;
            }
            break;
        case 32:
            //if((long long)Rs!=(long long)Rt) PC=temp_PC+Imm;
            if((long long)Rs==(long long)Rt)
            {
                bubble_flag[1]=bubble_flag[2]=1;
                PC=ID_EX.val_P;
            }
            break;
        case 33:
            //if((long long)Rs<(long long)Rt) PC=temp_PC+Imm;
            if((long long)Rs>=(long long)Rt)
            {
                bubble_flag[1]=bubble_flag[2]=1;
                PC=ID_EX.val_P;
            }
            break;
        case 34:
            //if((long long)Rs>=(long long)Rt) PC=temp_PC+Imm;
            if((long long)Rs<(long long)Rt)
            {
                bubble_flag[1]=bubble_flag[2]=1;
                PC=ID_EX.val_P;
            }
            break;
        case 35:
            ALUout=temp_PC+Imm;
            break;
        case 36:
            ALUout=Imm;
            break;
        case 37:
            ALUout=temp_PC+4;
            //PC=temp_PC+Imm;
            break;
        case 38://sra
            ALUout=Rs>>Rt;
            break;
        case 39://srai
            ALUout=Rs>>Imm;
            break;
        case 40://mulw
            ALUout=ext_signed((Rs*Rt)&0xffffffff,1,32);
            break;
        case 41://slliw
            ALUout=ext_signed((Rs<<Imm)&0xffffffff,0,32);
            break;
        case 42://srliw
            ALUout=ext_signed(((unsigned long long)Rs>>Imm)&0xffffffff,0,32);
            break;
        case 43://sraiw
            ALUout=ext_signed((Rs>>Imm)&0xffffffff,0,32);
            break;
        case 44://addw
            ALUout=ext_signed((Rs+Rt)&0xffffffff,0,32);
            break;
        default:
            dbg_printf("Invalid instruction\n");
            break;
    }

    if(ALUop == 8) div_flag = true;
    else if(ALUop == 11) rem_flag = true;
    else div_flag = rem_flag = false;
    if(ALUop==2||ALUop==5) mul_flag = true;
    else mul_flag = false;
/*
    //choose reg dst address
    int Reg_Dst;
    if(RegDst)
    {
        Reg_Dst=rd;
    }
    else
    {
        Reg_Dst=rt;
    }
*/
    //write EX_MEM_old
    EX_MEM_old.PC=temp_PC;
    EX_MEM_old.Reg_dst=ID_EX.Reg_dst;
    EX_MEM_old.ALU_out=ALUout;
    EX_MEM_old.Reg_Rt=Rt;

    EX_MEM_old.Ctrl_EX_ALUOp=ALUop;
    EX_MEM_old.Ctrl_M_Branch=ID_EX.Ctrl_M_Branch;
    EX_MEM_old.Ctrl_M_MemWrite=ID_EX.Ctrl_M_MemWrite;
    EX_MEM_old.Ctrl_M_MemRead=ID_EX.Ctrl_M_MemRead;

    EX_MEM_old.Ctrl_WB_RegWrite=ID_EX.Ctrl_WB_RegWrite;
    EX_MEM_old.Ctrl_WB_MemtoReg=ID_EX.Ctrl_WB_MemtoReg;
#ifdef DEBUG
    {
        dbg_printf("EX finished\n");
        print_EXMEM();
    }
#endif
}

//访存
    void MEM()
{
    dbg_printf("-------------MEM-------------\n");

    //read EX_MEM
    char Branch=EX_MEM.Ctrl_M_Branch;
    char MemWrite=EX_MEM.Ctrl_M_MemWrite;
    char MemRead=EX_MEM.Ctrl_M_MemRead;
    char ALUop=EX_MEM.Ctrl_EX_ALUOp;

    unsigned long long addr=EX_MEM.ALU_out;
    long long reg_rt=EX_MEM.Reg_Rt;

    long long val=0;

    if(MemRead == 1)
    {
        switch(ALUop)
        {
            case 13:
                memcpy(&val,&memory[addr],1);
                val=ext_signed(val,1,8);
                break;
            case 14:
                memcpy(&val,&memory[addr],2);
                val=ext_signed(val,1,16);
                break;
            case 15:
                memcpy(&val,&memory[addr],4);
                val=ext_signed(val,1,32);
                break;
            case 16:
                memcpy(&val,&memory[addr],8);
                break;
            default:
                break;
        }

    }
    else if(MemWrite == 1)
    {
        switch(ALUop)
        {
            case 27:
                val=getbit64(reg_rt,56,63);
                dbg_printf("val = %llx\n",val);
                memcpy(&memory[addr],&val,1);
                break;
            case 28:
                val=getbit64(reg_rt,48,63);
                dbg_printf("val = %llx\n",val);
                memcpy(&memory[addr],&val,2);
                break;
            case 29:
                val=getbit64(reg_rt,32,63);
                dbg_printf("val = %llx\n",val);
                memcpy(&memory[addr],&val,4);
                break;
            case 30:
                val=getbit64(reg_rt,0,63);
                dbg_printf("val = %llx\n",val);
                memcpy(&memory[addr],&val,8);
                break;
            default:
                break;
        }
    }
    //write MEM_WB_old
    if(MemRead == 0)
        MEM_WB_old.ALU_out=EX_MEM.ALU_out;
    else MEM_WB_old.ALU_out=val;
    MEM_WB_old.Mem_read=EX_MEM.Ctrl_M_MemRead;
    MEM_WB_old.Reg_dst=EX_MEM.Reg_dst;

    MEM_WB_old.Ctrl_WB_MemtoReg=EX_MEM.Ctrl_WB_MemtoReg;
    MEM_WB_old.Ctrl_WB_RegWrite=EX_MEM.Ctrl_WB_RegWrite;
#ifdef DEBUG
    {
        dbg_printf("MEM finished\n");
        print_MEMWB();
    }
#endif
}


//写回
void WB()
{
    dbg_printf("-------------WB--------------\n");
    //read MEM_WB
    char Mem_read=MEM_WB.Mem_read;
    REG ALU_out=MEM_WB.ALU_out;
    int Reg_dst=MEM_WB.Reg_dst;

    char Ctrl_WB_RegWrite=MEM_WB.Ctrl_WB_RegWrite;
    char Ctrl_WB_MemtoReg=MEM_WB.Ctrl_WB_MemtoReg;

    if(Ctrl_WB_RegWrite==1) reg[Reg_dst]=ALU_out;
    dbg_printf("WB finished\n");
    //instruction reaches WB then update inst_num
    inst_num++;
}
