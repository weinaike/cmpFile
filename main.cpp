#include <iostream>
#include <dirent.h>
#include <fstream>
#include <vector>
#include "json/json.h"

using namespace std;

map <string, string> wordsmap;


struct object
{
    int rect[4];
    string objName ;
};

string GetExt(string filename)
{
    string::size_type pos;
    pos = filename.find('.');
    string ext = filename.substr(pos+1);
    return ext;
}

string GetName(string filename)
{
    string::size_type pos;
    pos = filename.find('.');
    string name = filename.substr(0,pos);
    return name;
}

vector<object> ObjectFromFile(string path, string filename, string ext)
{
    vector<object> objset;
    string filepath = path + filename + '.' + ext;
    ifstream ifs(filepath);
    if(ifs.is_open())
    {
        if(ext == "txt")
        {
            string buffer;
            getline(ifs,buffer);
            int num = stoi(buffer);
            while(getline(ifs,buffer))
            {
                object obj;
                string::size_type pos;
                for(int i = 0; i < 5; i++)
                {
                    if(i == 4)
                    {
                        pos = buffer.find('\r');
                        obj.objName = buffer.substr(0,pos);
                    }
                    else
                    {
                        pos = buffer.find(' ');
                        obj.rect[i] = stoi(buffer.substr(0,pos));
                        buffer = buffer.substr(pos+1);
                    }
                }
                objset.push_back(obj);
            }
        }
        else if(ext == "json")
        {
            Json::Reader reader;
            Json::Value root;
            if(reader.parse(ifs,root))
            {
                //读取根节点信息
                Json::Value subroot = root["objects"];
                int num = root["number"].asInt();
                //读取子节点信息
                for (int i = 0; i < num; i++)
                {
                    object obj;
                    string name = subroot[i]["name"].asString();
                    float bottom = subroot[i]["bottom"].asFloat();
                    float left = subroot[i]["left"].asFloat();
                    float right = subroot[i]["right"].asFloat();
                    float top = subroot[i]["top"].asFloat();
                    obj.rect[0] = int (left * 1280) ;
                    obj.rect[1] = int (top * 720) ;
                    obj.rect[2] = int (right * 1280) ;
                    obj.rect[3] = int (bottom * 720) ;

                    map <string, string>::iterator itr;
                    itr = wordsmap.find(name);
                    obj.objName = itr->second;
                    objset.push_back(obj);
                }
            }
            ifs.close();
        }
        else
        {
            cout<<"unknown file type"<<endl;

        }
    }
    else
    {
        cout<<filepath<<" is not exist"<<endl;

    }
    return objset;

}

float CalcAera(int box1[],int box2[])
{
    float s_i,s_j,xmin,ymin,xmax,ymax,w,h,inters,overlap;
    s_i=(box1[2]-box1[0]+1.0)*(box1[3]-box1[1]+1.0);
    s_j=(box2[2]-box2[0]+1.0)*(box2[3]-box2[1]+1.0);
    xmin = max(box1[0], box2[0]);
    ymin = max(box1[1], box2[1]);
    xmax = min(box1[2], box2[2]);
    ymax = min(box1[3], box2[3]);
    w = max(xmax - xmin + 1., 0.);
    h = max(ymax - ymin + 1., 0.);
    inters = w * h;
    overlap=inters/(s_i+s_j-inters);
    return overlap;

}

bool IsOverlap(object obj1,object obj2)
{
    if((obj1.objName == obj2.objName)&&(CalcAera(obj1.rect,obj2.rect)>=0.5))
    {
        return true;
    }
    else
    {
        return false;
    }
}


int main(int argc, char *argv[])
{
    struct dirent *fileinfo = NULL;
    char path[] = "/home/wnk/Desktop/side/";
    char path2[] = "/home/wnk/Desktop/detect/";

    // load words map : pingyin vs chinese
    ifstream ifs("/home/wnk/qtwork/cmpFile/wordsmap.cfg");
    map <string, vector<int>> resultmap;
    if(ifs.is_open())
    {
        string buffer;
        while(getline(ifs,buffer))
        {
            string::size_type pos ;
            pos = buffer.find(' ',0);
            wordsmap.insert(pair <string,string>(buffer.substr(0,pos),buffer.substr(pos+1)));
            vector<int> result;
            result.push_back(0);//ground truth
            result.push_back(0);//detected num
            result.push_back(0);//detected is right
            result.push_back(0);//ground truth is right detected
            resultmap.insert(pair <string,vector<int>>(buffer.substr(pos+1),result));

        }
    }
    else
    {
        cout<<"load words map is false"<<endl;
        return 0;
    }


    DIR *pdir = opendir(path);
    int numFile = 0;
    while(NULL != (fileinfo = readdir(pdir)))
    {
        numFile++;
        if(fileinfo->d_name[0] == '.')
            continue;
        string fullname = fileinfo->d_name;
        string ext = GetExt(fullname);
        vector<object> gtobj,ptobj;
        if(ext == "txt")
        {
            gtobj = ObjectFromFile(path, GetName(fullname),"txt");
            ptobj = ObjectFromFile(path2, GetName(fullname),"json");
        }

        cout<<"~~~~~~~~~~~~~~~~~~~~"<<numFile<<"~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
        cout<<"~~~ground truth is "<< gtobj.size()<<"~~~"<<endl;
        for (unsigned i = 0; i < gtobj.size(); i++)
        {
            for(int j = 0; j < 4; j++)
            {
                cout<<gtobj[i].rect[j]<<" ";
            }
            cout<<gtobj[i].objName<<endl;
        }

        cout<<"~~~detected num is "<<ptobj.size()<<"~~~"<<endl;
        for (unsigned i = 0; i < ptobj.size(); i++)
        {
            for(int j = 0; j < 4; j++)
            {
                cout<<ptobj[i].rect[j]<<" ";
            }
            cout<<ptobj[i].objName<<endl;
        }


        int numMatch = 0;
        for (unsigned i = 0; i < gtobj.size(); i++)
        {
            int flag = 0;
            map <string,vector<int>>::iterator gtitr;
            gtitr = resultmap.find(gtobj[i].objName);
            gtitr->second[0]++;
            for (unsigned j = 0; j < ptobj.size(); j++)
            {
                map <string,vector<int>>::iterator ptitr;
                if(i == 0)
                {
                    ptitr = resultmap.find(ptobj[j].objName);
                    ptitr->second[1]++;
                }
                if(IsOverlap(gtobj[i],ptobj[j]))
                {
                    ptitr = resultmap.find(gtobj[i].objName);
                    ptitr->second[2]++;
                    if(flag == 0)
                    {
                        numMatch++;
                        ptitr->second[3]++;
                        flag = 1;
                    }

                }
            }
        }
        cout<<"~~~right result is "<<numMatch<<"~~~"<<endl;

    }
    closedir(pdir);

    cout<<"~~~~~~~~~~~~~~~~~all result~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    map <string, vector<int>>::iterator itr;
    float recall_all = 0;
    float precise_all = 0;
    int num_clac = 0;
    for(itr = resultmap.begin(); itr!=resultmap.end(); itr++)
    {
        cout<<itr->first<<" ";
        for(int i = 0; i < 4; i++)
        {
            cout<<itr->second[i]<<" ";
        }

        if(itr->second[1] != 0)
        {
            recall_all += float(itr->second[3])/itr->second[0];
            precise_all += float(itr->second[2])/itr->second[1];
            cout<<float(itr->second[3])/itr->second[0]<<" ";
            cout<<float(itr->second[2])/itr->second[1]<<" ";
            cout<<endl;
            num_clac++;
        }
        else
        {
            cout<<endl;
        }
    }
    cout<<"calc num "<<num_clac<<endl;
    cout<<"avrage recall: "<<recall_all/num_clac<<endl;
    cout<<"avrage precise: "<<precise_all/num_clac<<endl;

    return 0;
}



