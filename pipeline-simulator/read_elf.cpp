#include"read_elf.h"

FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//Program headers
long long padr=0;
unsigned short psize=0;
unsigned int pnum=0;

//Section Headers
long long sadr=0;
unsigned short ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
long long symadr=0;
long long symsize=0;
long long symensize=0;

//用于指示包含节名称的字符串是第几个节(从0开始计数)
unsigned int Index=0;

//字符串表在文件中地址，其内容包括.symtab和.debug节中的符号表
long long stradr=0;
long long shstradr=0;

char FILENAME[20];

bool open_file()
{
    
    printf("Please input the file name:");
    cin >> FILENAME;
    cout<<FILENAME<<endl;

    file=fopen(FILENAME,"r");
    elf=fopen("elf.txt","w");
    if(file==NULL) return false;
    return true;
}

bool read_elf()
{
    bsize=0;
    csize=0;
    dsize=0;
    if(!open_file()){
        cout<<"file error" << endl;
        return false;
    }

    fprintf(elf,"ELF Header:\n");
    read_Elf_header();

    fprintf(elf,"\n\nSection Headers:\n");
    read_elf_sections();

    fprintf(elf,"\n\nProgram Headers:\n");
    read_Phdr();

    fprintf(elf,"\n\nSymbol table:\n");
    read_symtable();

    fclose(elf);
    return true;
}

void read_Elf_header()
{
    //file should be relocated
    fread(&elf64_hdr,1,sizeof(elf64_hdr),file);

    fprintf(elf," magic number: %x %x %x %x \n",elf64_hdr.e_ident[0],elf64_hdr.e_ident[1],elf64_hdr.e_ident[2],elf64_hdr.e_ident[3]);

    switch(elf64_hdr.e_ident[4]){
        case 0:
            fprintf(elf," Class:  ILLEGAL ELFCLASS\n");
            break;
        case 1:
            fprintf(elf," Class:  ELFCLASS32\n");
            break;
        case 2:
            fprintf(elf," Class:  ELFCLASS64\n");
            break;
        default:
            break;
    }

    switch(elf64_hdr.e_ident[5]){
        case 0:
            fprintf(elf," Data:  ILLEGAL\n");
            break;
        case 1:
            fprintf(elf," Data:  little-endian\n");
            break;
        case 2:
            fprintf(elf," Data:  big-endian\n");
            break;
        default:
            break;
    }

    fprintf(elf," Version:  current\n");

    fprintf(elf," OS/ABI:	 System V ABI\n");

    fprintf(elf," ABI Version:   \n");

    fprintf(elf," Type: 0x%x \n", elf64_hdr.e_type);

    fprintf(elf," Machine: 0x%x  \n", elf64_hdr.e_machine);

    fprintf(elf," Version: 0x%x \n",elf64_hdr.e_version);

    memcpy(&vcadr,&elf64_hdr.e_entry,sizeof(elf64_hdr.e_entry));
    fprintf(elf," Entry point address:  0x%x\n",elf64_hdr.e_entry);

    memcpy(&padr,&elf64_hdr.e_phoff,sizeof(elf64_hdr.e_phoff));
    fprintf(elf," Start of program headers: %ld bytes into file\n", elf64_hdr.e_phoff);

    memcpy(&sadr,&elf64_hdr.e_shoff,sizeof(elf64_hdr.e_shoff));
    fprintf(elf," Start of section headers: %ld bytes into file\n",elf64_hdr.e_shoff);

    fprintf(elf," Flags:  0x%x\n",elf64_hdr.e_flags);

    fprintf(elf," Size of this header: %hd bytes\n",elf64_hdr.e_ehsize);

    memcpy(&psize,&elf64_hdr.e_phentsize,sizeof(elf64_hdr.e_phentsize));
    fprintf(elf," Size of program headers: %hd bytes\n",elf64_hdr.e_phentsize);

    memcpy(&pnum,&elf64_hdr.e_phnum,sizeof(elf64_hdr.e_phnum));
    fprintf(elf," Number of program headers: %hd \n",elf64_hdr.e_phnum);

    memcpy(&ssize,&elf64_hdr.e_shentsize,sizeof(elf64_hdr.e_shentsize));
    fprintf(elf," Size of section headers: %hd bytes\n",elf64_hdr.e_shentsize);

    memcpy(&snum,&elf64_hdr.e_shnum.b,2);
    fprintf(elf," Number of section headers: %hd \n",elf64_hdr.e_shnum);

    memcpy(&shstradr,&elf64_hdr.e_shstrndx,sizeof(elf64_hdr.e_shstrndx));
    fprintf(elf," Section header string table index: %hd \n",elf64_hdr.e_shstrndx);
}

void read_elf_sections()
{

    Elf64_Shdr elf64_shdr;
    shstradr=sadr+shstradr*ssize;

    //file should be relocated
    fseek(file,shstradr,SEEK_SET);
    fread(&elf64_shdr,1,sizeof(elf64_shdr),file);
    memcpy(&shstradr,&elf64_shdr.sh_offset,sizeof(elf64_shdr.sh_offset));

    for(int c=0;c<snum;c++)
    {
        fseek(file,sadr+c*ssize,SEEK_SET);
        fprintf(elf," [%3d]\n",c);

        //file should be relocated
        fread(&elf64_shdr,1,sizeof(elf64_shdr),file);
        memcpy(&Index,&elf64_shdr.sh_name,sizeof(elf64_shdr.sh_name));

        fseek(file,shstradr+Index,SEEK_SET);

        char section_name[20];
        char temp;

        int i=0;
        while(1)
        {
            fread(&temp,1,1,file);
            if(temp!='\0') section_name[i++]=temp;
            else break;
        }
        section_name[i]='\0';


        if(strcmp(section_name,".text")==0)
        {
            memcpy(&cadr, &elf64_shdr.sh_offset, sizeof(elf64_shdr.sh_offset));
            memcpy(&csize, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
        }
        else if(strcmp(section_name,".rodata")==0)
        {
            long long temp_size;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            csize += temp_size;
        }
        else if(strcmp(section_name,".init_array")==0)
        {
            long long temp_size = 0;
            memcpy(&dadr, &elf64_shdr.sh_offset, sizeof(elf64_shdr.sh_offset));
            memcpy(&vdadr, &elf64_shdr.sh_addr, sizeof(elf64_shdr.sh_addr));
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".fini_array")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".eh_frame")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".jcr")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".data")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".sdata")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
        }
        else if(strcmp(section_name,".bss")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
            bsize += temp_size;
        }
        else if(strcmp(section_name,".sbss")==0)
        {
            long long temp_size = 0;
            memcpy(&temp_size, &elf64_shdr.sh_size, sizeof(elf64_shdr.sh_size));
            dsize += temp_size;
            bsize += temp_size;
        }
        else if(strcmp(section_name,".symtab")==0)
        {
            memcpy(&symadr,&elf64_shdr.sh_offset,sizeof(elf64_shdr.sh_offset));
            memcpy(&symsize,&elf64_shdr.sh_size,sizeof(elf64_shdr.sh_size));
            memcpy(&symensize,&elf64_shdr.sh_entsize,sizeof(elf64_shdr.sh_entsize));
            symnum=symsize/symensize;
        }
        else if(strcmp(section_name,".strtab")==0)
        {
            memcpy(&stradr,&elf64_shdr.sh_offset,sizeof(elf64_shdr.sh_offset));
        }


        fprintf(elf," Name: %s",section_name);

        fprintf(elf," Type: %x",&elf64_shdr.sh_type);

        fprintf(elf," Address: %x ",elf64_shdr.sh_addr);

        fprintf(elf," Offest: %x \n",elf64_shdr.sh_offset);

        fprintf(elf," Size: %x ",elf64_shdr.sh_size);

        fprintf(elf," Entsize: %x ",elf64_shdr.sh_entsize);

        fprintf(elf," Flags: %x  ",&elf64_shdr.sh_flags);

        fprintf(elf," Link: %d ",elf64_shdr.sh_link);

        fprintf(elf," Info: %d ",elf64_shdr.sh_info);

        fprintf(elf," Align: %ld\n",elf64_shdr.sh_addralign);
    }
}

void read_Phdr()
{
    Elf64_Phdr elf64_phdr;

    for(int c=0;c<pnum;c++)
    {
        fseek(file,padr+c*psize,SEEK_SET);
        fprintf(elf," [%3d]\n",c);

        //file should be relocated
        fread(&elf64_phdr,1,sizeof(elf64_phdr),file);

        fprintf(elf," Type: 0x%d  ",elf64_phdr.p_type);

        fprintf(elf," Flags: 0x%x  ",elf64_phdr.p_flags);

        fprintf(elf," Offset: 0x%x  ",elf64_phdr.p_offset);

        fprintf(elf," VirtAddr: 0x%x ",elf64_phdr.p_vaddr);

        fprintf(elf," PhysAddr:  0x%x ",elf64_phdr.p_paddr);

        fprintf(elf," FileSiz:  0x%x ",elf64_phdr.p_filesz);

        fprintf(elf," MemSiz: 0x%x ",elf64_phdr.p_memsz);

        fprintf(elf," Align: 0x%x  \n",elf64_phdr.p_align);
    }
}


void read_symtable()
{
    Elf64_Sym elf64_sym;

    for(int c=0;c<symnum;c++)
    {
        fprintf(elf," [%3d]   ",c);

        //file should be relocated
        fseek(file,symadr+c*symensize,SEEK_SET);
        fread(&elf64_sym,1,sizeof(elf64_sym),file);

        memcpy(&Index,&elf64_sym.st_name,sizeof(elf64_sym.st_name));

        fseek(file,stradr+Index,SEEK_SET);

        char symtable_name[40];
        char temp;

        int i=0;
        while(1)
        {
            fread(&temp,1,1,file);
            if(temp!='\0') symtable_name[i++]=temp;
            else break;
        }
        symtable_name[i]='\0';

        if(strcmp(symtable_name,"main")==0)
            {
                memcpy(&madr,&elf64_sym.st_value,sizeof(elf64_sym.st_value));
                memcpy(&msize,&elf64_sym.st_size,sizeof(elf64_sym.st_size));
            }
        else if(strcmp(symtable_name,"__global_pointer$")==0 || strcmp(symtable_name,"_gp")==0)
            memcpy(&gp,&elf64_sym.st_value,sizeof(elf64_sym.st_value));

        fprintf(elf," Name:  %40s   ", symtable_name);

        fprintf(elf," Bind:  0x%x ",elf64_sym.st_info);

        fprintf(elf," Type: 0x%x  ",elf64_sym.st_other);

        fprintf(elf," NDX: 0x%x  ",elf64_sym.st_shndx);

        fprintf(elf," Size: 0x%x  ",elf64_sym.st_size);

        fprintf(elf," Value: 0x%x \n",elf64_sym.st_value);
    }

    fprintf(elf, "main address = 0x%x\n", madr);
}
