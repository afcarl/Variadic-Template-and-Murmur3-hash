
#ifndef MURMUR3_H
#define MURMUR3_H

#include <iostream>
#include <tuple>
#include <vector>
#include <iomanip>
#include <random>
#include <type_traits>
using namespace std;

namespace MUR
{
    //Utility class which is used to convert a variadic tuple to a bytearray
    class TupleToByteArrayC
    {

        public:

            //used to store the byte representation of each key in the tuple
            static vector<uint8_t> bytearray;

            template<class... Args>
            static void Unpack(const std::tuple<Args...>& kt)
            {
                TupleUnpack<decltype(kt), sizeof...(Args)>::Unpack(kt);
            }

        private:

            template <typename C>
            static void ToByteUtil(const C& kdata)
            {
                uint8_t* pbyte = const_cast<uint8_t*>(reinterpret_cast<const uint8_t *>(&kdata));
                bytearray.insert(bytearray.end(),pbyte,pbyte+ sizeof(kdata));

            }

            static void ToByte(const wstring & kdata)
            {
                 cout<<"C4";
                 for(size_t i = 0 ; i<kdata.length() ;i++)
                 {
                     ToByteUtil(kdata[i]);
                 }
            }

            static void ToByte(const string & kdata)
            {
                  cout<<"C3";
                  for(size_t i = 0 ; i<kdata.length() ;i++)
                  {
                     ToByteUtil(kdata[i]);
                  }
            }



            template<typename T>
            static void ToByte(const T& kdata,typename std::enable_if<std::is_pointer<T>::value >::type* = 0)
            {
               cout<<"C2";
               for(int i = 0 ; kdata[i] != L'\0' || kdata[i] !='\0' ;i++)
               {
                    ToByteUtil(kdata[i]);
               }
            }


            template<typename T>
            static void ToByte(const T&kdata,typename std::enable_if<!std::is_pointer<T>::value >::type* = 0)
            {
                  cout<<"C1"<<endl;
                  ToByteUtil(kdata);
            }


            template<class Tuple, std::size_t N>
            struct TupleUnpack
            {
                static void Unpack(const Tuple& kt)
                {
                    TupleUnpack<Tuple, N-1>::Unpack(kt);
                    cout<<"Tuplevalue"<<N-1<<"="<<std::get<N-1>(kt)<<endl;
                    ToByte(std::get<N-1>(kt));
                }
            };

            template<class Tuple>
            struct TupleUnpack<Tuple, 1>
            {
                static void Unpack(const Tuple& kt)
                {
                    cout<<"Tuplevalue0="<<std::get<0>(kt)<<endl;
                    ToByte(std::get<0>(kt));
                }
            };

    };

    //Initializng the static member of the above class
    vector<uint8_t>TupleToByteArrayC::bytearray;

    //Variadic template class used to implement hashing of multiple keys to a single value
    template<typename... Arguments>
    class Murmur3
    {
        public:
            Murmur3()
            {

            }
            ~Murmur3()
            {

            }


            //This method takes the lambda function and the parameter pack as input and
            //returns the 32 bit hash value
            template<typename Func>
            uint32_t apply(Func&& lambdafunc,const Arguments&...kparams)
            {
                std::tuple<Arguments...>t = lambdafunc(kparams...);
                return ComputeHashUsingInputTuple(t);
            }


        private:

            //This method is used by the below method to rotate a value left by specified number of places.
            uint32_t RotateLeft(uint32_t value,uint32_t numberofplaces)
            {
                return (value << numberofplaces)|(value >>(32 -numberofplaces));
            }


            //This method implements Murmur3 Hash algorithm.
            //The byte array,its length and the seed are the input parameters
            uint32_t CalculateHashedValue(const uint8_t*kbytearray,size_t len,uint32_t seed)
            {

                uint32_t hash = 0;
                if(kbytearray != NULL)
                {
                    //Constant values used in the below algorithm
                    const uint32_t kc1 = 0xcc9e2d51;
                    const uint32_t kc2 = 0x1b873593;
                    const uint32_t kr1 = 15;
                    const uint32_t kr2 = 13;
                    const uint32_t kr3 = 16;
                    const uint32_t km = 5;
                    const uint32_t kn = 0xe6546b64;
                    const uint32_t kc3 = 0x85ebca6b;
                    const uint32_t kc4 = 0xc2b2ae35;

                    hash = seed;


                    //if the length of Byte Array is greater than 4 bytes
                    //suceessive chunk of 4 bytes is considered at once in each iteration
                    if(len>3)
                    {
                        const uint32_t*bytechunk = (const uint32_t*)kbytearray;

                        //Getting the maximum possible number of multiples of 4
                        size_t i = len >>2;

                        do
                        {
                            uint32_t fourbytekey = *bytechunk++;
                            fourbytekey *= kc1;

                            //Four byte key is Left rotated by 15 places
                            fourbytekey = RotateLeft(fourbytekey,kr1);

                            fourbytekey *= kc2;
                            hash ^= fourbytekey;

                            //Hash value is Left rotated by 13 places
                            hash = RotateLeft(hash,kr2);

                            hash = (hash*km)+kn;

                        }while(--i);

                        //The byte values upto the possible largest multiple of four are considered
                        //for hash calculation. If present, remaining bytes have to be considered
                        kbytearray = (const uint8_t*)bytechunk;
                    }

                    //Below block of code would be executed if byte array length
                    //is less than four or if remaining bytes are present
                    if (len & 3)
                    {
                        size_t i = len & 3;
                        uint32_t rembytekey = 0;

                        //Consider the address of the last byte
                        kbytearray = &kbytearray[i - 1];

                        //Remaining bytes are concatenated
                        // in reverse order starting from last position
                        do
                        {
                            rembytekey <<= 8;
                            rembytekey |= *kbytearray--;
                        } while (--i);

                        rembytekey *= kc1;

                        //Key formed with remaining bytes is Left rotated by 15 places
                        rembytekey = RotateLeft(rembytekey,kr1);

                        rembytekey *= kc2;
                        hash ^= rembytekey;
                    }

                    //Further bit manipulations on hash value
                    hash ^= len;
                    hash ^= (hash >> kr3);
                    hash *= kc3;
                    hash ^= (hash >> kr2);
                    hash *= kc4;
                    hash ^= (hash >> kr3);

                }

                return hash;
            }

            //This method returns 32bit hash value by taking tuple as input
            uint32_t ComputeHashUsingInputTuple(const tuple<Arguments...> &kt)
            {

                //Unpacking the tuple to a byte array using the Utility class declared above
                TupleToByteArrayC::Unpack(kt);
                cout<<endl<<"ByteArraySize="<<TupleToByteArrayC::bytearray.size()<<endl;

                //Printing the bytearray
                cout<<"ByteArrayvalues:"<<endl;
                for(size_t i = 0 ;i<TupleToByteArrayC::bytearray.size();++i)
                {
                    cout<<hex<<(int)TupleToByteArrayC::bytearray[i]<<" ";
                }
                cout<<endl;

                //Generation of 32 bit random integer
                random_device rd;
                mt19937_64 gen(rd());
                uniform_int_distribution<uint32_t> dis;
                uint32_t randomseed = dis(gen);
                cout<<endl<<"RandomSeed="<<randomseed<<endl;

                uint8_t*pbyte = NULL;
                if(!TupleToByteArrayC::bytearray.empty())
                {
                    pbyte = &TupleToByteArrayC::bytearray[0];
                }

                return CalculateHashedValue(pbyte,TupleToByteArrayC::bytearray.size(),randomseed);

            }
    };
}
#endif // MURMUR3_H
