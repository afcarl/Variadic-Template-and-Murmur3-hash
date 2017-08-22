#include"Murmur3.h"
using namespace MUR;

int main()
{
    //variadic generic lambda to facilitate input of multiple parameters of different types
    auto lambdafunc = [](const auto& ...params)
    {
        return std::make_tuple(params...);
    };

    Murmur3<int,float,double,uint32_t,string,char*,char*,double,float,int> obj;
    uint32_t hashvalue = obj.apply(lambdafunc,10,3.14f,5.4,5,"murmur0123456989","pq","jr",-50.4,-30.14f,-100);


    cout<<endl<<"Hash Value for Given Input ="<<hashvalue<<endl;

    return 0;
}
