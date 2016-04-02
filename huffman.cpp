#include<iostream>
#include<fstream>

using namespace std;

struct HTNode{
    int weight;
    int parent,lchild,rchild;
    HTNode():weight(0),parent(-1),lchild(-1),rchild(-1){}
};

class obinstream{
public:
    obinstream(ofstream &outfile):outfile(outfile),buffp(0),buff(0){}
    void writebit(int bit){
        if(bit!=0&&bit!=1)throw 0;
        buff|=bit<<buffp;
        buffp++;
        if(buffp==8){
            flush();
        }
    }
    void flush(){
        outfile.write((char*)&buff,1);
        buff=0;
        buffp=0;
    }
    void writebits(string s){
        for(int i=0;i<s.size();i++){
            writebit(s[i]-'0');
        }
    }
private:
    ofstream &outfile;
    uint8_t buff;
    int buffp;
};

class ibinstream{
public:
    ibinstream(ifstream &infile,HTNode *HT,int root):infile(infile),HT(HT),root(root),buffp(0),buff(0){}
    int readbit(){
        if(buffp==0){
            infile.read((char*)&buff,1);
        }
        int bit=(buff>>buffp)&1;
        buffp++;
        buffp%=8;
        return bit;
    }
    uint8_t readchar(){
        int p;
        for(p=root;HT[p].lchild>=0||HT[p].rchild>=0;p=readbit()?HT[p].rchild:HT[p].lchild);
        return p;
    }
private:
    ifstream &infile;
    HTNode *HT;
    int root;
    uint8_t buff;
    int buffp;
};

void huffmancode(HTNode *HT,string *code,int root,string prefix=""){
    if(HT[root].lchild<0&&HT[root].rchild<0){
        code[root]=prefix;
    }else{
        if(HT[root].lchild>=0){
            huffmancode(HT,code,HT[root].lchild,prefix+"0");
        }
        if(HT[root].rchild>=0){
            huffmancode(HT,code,HT[root].rchild,prefix+"1");
        }
    }
}

void compress(ifstream &infile,ofstream &outfile){
    HTNode HT[511];
    uint8_t c;
    while(!infile.eof()){
        infile.read((char*)&c,1);
        HT[c].weight++;
    }

    for(int i=256;i<511;i++){
        int m1,m2,p1=-1,p2=-1;
        for(int j=0;j<i;j++){
            if(HT[j].parent<0){
                if(p1<0||HT[j].weight<m1){
                    m2=m1;
                    p2=p1;
                    m1=HT[j].weight;
                    p1=j;
                }else if(p2<0||HT[j].weight<m2){
                    m2=HT[j].weight;
                    p2=j;
                }
            }
        }
        if(p1>p2){
            swap(p1,p2);
            swap(m1,m2);
        }
        HT[i].weight=HT[p1].weight+HT[p2].weight;
        HT[i].lchild=p1;
        HT[i].rchild=p2;
        HT[p1].parent=i;
        HT[p2].parent=i;
    }

    string code[256];
    huffmancode(HT,code,510);
    /*for(int i=0;i<512;i++){
        cout<<i<<" "<<(char)i<<" "<<HT[i].weight<<" "<<code[i]<<" "<<HT[i].parent<<" "<<HT[i].lchild<<" "<<HT[i].rchild<<endl;
    }*/

    outfile<<"ZZH"<<'\0';
    infile.clear();
    uint32_t filesize=infile.tellg();
    outfile.write((char*)&filesize,4);
    for(int i=0;i<510;i++){
        c=HT[i].parent&255;
        outfile.write((char*)&c,1);
    }

    infile.seekg(0);
    obinstream obin(outfile);
    while(!infile.eof()){
        infile.read((char*)&c,1);
        obin.writebits(code[c]);
    }
    obin.flush();
}

void uncompress(ifstream &infile,ofstream &outfile){
    char filehead[4];
    infile.read(filehead,4);
    for(int i=0;i<4;i++){
        if(filehead[i]!="ZZH"[i]){
            cout<<"文件类型错误！"<<endl;
            return;
        }
    }
    uint32_t filesize;
    infile.read((char*)&filesize,4);
    HTNode HT[511];
    uint8_t c;
    for(int i=0;i<510;i++){
        infile.read((char*)&c,1);
        int p=c+256;
        HT[i].parent=p;
        if(HT[p].lchild<0){
            HT[p].lchild=i;
        }else{
            HT[p].rchild=i;
            if(HT[p].lchild>HT[p].rchild){
                swap(HT[p].lchild,HT[p].rchild);
            }
        }
    }

    /*for(int i=0;i<512;i++){
        cout<<i<<" "<<(char)i<<" "<<HT[i].weight<<" "<<HT[i].parent<<" "<<HT[i].lchild<<" "<<HT[i].rchild<<endl;
    }*/

    ibinstream ibin(infile,HT,510);
    for(int i=0;i<filesize;i++){
        c=ibin.readchar();
        outfile.write((char*)&c,1);
    }
}

int main(int argc,char **argv){
    if(argc<4){
        cout<<"Usage: "<<argv[0]<<" [选项] [源文件] [目标文件]"<<endl;
        cout<<"  -c, --compress   压缩"<<endl;
        cout<<"  -u, --uncompress 解压缩"<<endl;
        return 0;
    }
    string oper(argv[1]);
    ifstream infile(argv[2],std::ios::binary);
    if(!infile){
        cout<<"打开源文件出错"<<endl;
        return 0;
    }
    ofstream outfile(argv[3],std::ios::binary);
    if(!outfile){
        cout<<"打开目标文件出错"<<endl;
        return 0;
    }

    if(oper=="-c"||oper=="--compress"){
        compress(infile,outfile);
    }else if(oper=="-u"||oper=="--uncompress"){
        uncompress(infile,outfile);
    }else{
        cout<<"未知参数: "<<oper<<endl;
    }
    infile.close();
    outfile.close();
    return 0;

}
