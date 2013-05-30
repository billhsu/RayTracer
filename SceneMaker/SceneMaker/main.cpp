#include <iostream>
#include <vector>

using namespace std;
struct vertex2d{float x,y;};
vector<vertex2d> vertexList;
int main()
{
    int vertex_cnt=0;
    float room_height=0.0f;
    cout<<"Room height:";
    cin>>room_height;
    cout<<"Vertex count:";
    cin>>vertex_cnt;

    for(int i=0;i<vertex_cnt;++i)
    {
        vertex2d v;
        cout<<"vertex["<<i<<"]:";
        cin>>v.x>>v.y;
        vertexList.push_back(v);
    }
    cout<<"OBJ out:\n\n";
    for (int i=0;i<vertex_cnt;++i)
    {
        cout<<"v "<<vertexList[i].x<<" "<<room_height/2.0f<<" "<<vertexList[i].y<<endl;
        cout<<"v "<<vertexList[i].x<<" "<<-room_height/2.0f<<" "<<vertexList[i].y<<endl;
    }
    cout<<"g Room"<<endl;
    for (int i=0;i<vertex_cnt-1;++i)
    {
        cout<<"f "<<2*i+1<<" "<<2*i+2<<" "<<2*i+4<<endl;
        cout<<"f "<<2*i+1<<" "<<2*i+3<<" "<<2*i+4<<endl;
    }
    int i=vertex_cnt-1;
    cout<<"f "<<2*i+1<<" "<<2*i+2<<" "<<2<<endl;
    cout<<"f "<<2*i+1<<" "<<1<<" "<<2<<endl;
    int topFaceCnt=vertex_cnt-2;
    for(int i=0;i<topFaceCnt;++i)
    {
        cout<<"f "<<1<<" "<<2*i+3<<" "<<2*i+5<<endl;
        cout<<"f "<<2<<" "<<2*i+4<<" "<<2*i+6<<endl;
    }
    system("pause");
}