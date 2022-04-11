/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */                                     ////__DACE:0
#include <dace/dace.h>                                                            ////__DACE:0
                                                                                  ////__DACE:0
void FOR26_2_3_0(int& ___w, int& ___h, float* ___imgIn, int _argcount, int i) {    ////__DACE:2:3:0
    long long j;                                                                      ////__DACE:3
    ////__DACE:3
    for (j = 0; (j < ___h); j = j+1) {                                                ////__DACE:3:2
        {                                                                             ////__DACE:3:3
            
            
            {                                                                         ////__DACE:3:3:1
                float imgIn;                                                                      ////__DACE:3:3:1    ////__DACE:3:3:1
                ////__DACE:3:3:1                                                      ////__DACE:3:3:1
                ///////////////////                                                               ////__DACE:3:3:1    ////__DACE:3:3:1
                imgIn=float)(((313*i+991*j)%65536)/65535.0f;                                      ////__DACE:3:3:1    ////__DACE:3:3:1
                ///////////////////                                                               ////__DACE:3:3:1    ////__DACE:3:3:1
                ////__DACE:3:3:1                                                      ////__DACE:3:3:1
                ___imgIn[((4096 * i) + j)] = imgIn;                                               ////__DACE:3:3:1    ////__DACE:3:3:1
            }                                                                         ////__DACE:3:3:1
        }                                                                             ////__DACE:3:3
    }                                                                                 ////__DACE:3:2
    
}                                                                                 ////__DACE:2:3:0
////__DACE:2:3:0
void init_array_1_0_4(int& __w, int& __h, float& __alpha, float* __imgIn, int _argcount) {    ////__DACE:1:0:4
    long long i;                                                                      ////__DACE:2
    ////__DACE:2
    {                                                                                 ////__DACE:2:0
        
        
        {                                                                             ////__DACE:2:0:0
            float alpha;                                                                      ////__DACE:2:0:0    ////__DACE:2:0:0
            ////__DACE:2:0:0                                                          ////__DACE:2:0:0
            ///////////////////                                                               ////__DACE:2:0:0    ////__DACE:2:0:0
            *alpha=0.25;                                                                      ////__DACE:2:0:0    ////__DACE:2:0:0
            ///////////////////                                                               ////__DACE:2:0:0    ////__DACE:2:0:0
            ////__DACE:2:0:0                                                          ////__DACE:2:0:0
            __alpha = alpha;                                                                  ////__DACE:2:0:0    ////__DACE:2:0:0
        }                                                                             ////__DACE:2:0:0
    }                                                                                 ////__DACE:2:0
    for (i = 0; (i < __w); i = i+1) {                                                 ////__DACE:2:2
        {                                                                             ////__DACE:2:3
            
            
            FOR26_2_3_0(__w, __h, &__imgIn[0], _argcount, i);                         ////__DACE:2:3:0
        }                                                                             ////__DACE:2:3
    }                                                                                 ////__DACE:2:2
    
}                                                                                 ////__DACE:1:0:4
////__DACE:1:0:4
void FOR87_5_3_0(int& ____h, float* ____imgIn, float* ____y1, float& ___xm1, float& ___ym1, float& ___ym2, float& ___a1, float& ___a2, float& ___b1, float& ___b2, int _argcount, int i, int j) {    ////__DACE:5:3:0
    ////__DACE:6
    {                                                                                 ////__DACE:6:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:6:0:0
                    float a1 = ___a1;                                                                 ////__DACE:6:0:1,0    ////__DACE:6:0:0
                    float imgIn = ____imgIn[((4096 * i) + j)];                                        ////__DACE:6:0:2,0    ////__DACE:6:0:0
                    float a2 = ___a2;                                                                 ////__DACE:6:0:3,0    ////__DACE:6:0:0
                    float xm1 = ___xm1;                                                               ////__DACE:6:0:4,0    ////__DACE:6:0:0
                    float b1 = ___b1;                                                                 ////__DACE:6:0:5,0    ////__DACE:6:0:0
                    float ym1 = ___ym1;                                                               ////__DACE:6:0:6,0    ////__DACE:6:0:0
                    float b2 = ___b2;                                                                 ////__DACE:6:0:7,0    ////__DACE:6:0:0
                    float ym2 = ___ym2;                                                               ////__DACE:6:0:8,0    ////__DACE:6:0:0
                    float y1;                                                                         ////__DACE:6:0:0    ////__DACE:6:0:0
                    ////__DACE:6:0:0                                                  ////__DACE:6:0:0
                    ///////////////////                                                               ////__DACE:6:0:0    ////__DACE:6:0:0
                    y1=a1*imgIn+a2*xm1+b1*ym1+b2*ym2;                                                 ////__DACE:6:0:0    ////__DACE:6:0:0
                    ///////////////////                                                               ////__DACE:6:0:0    ////__DACE:6:0:0
                    ////__DACE:6:0:0                                                  ////__DACE:6:0:0
                    ____y1[((4096 * i) + j)] = y1;                                                    ////__DACE:6:0:0    ////__DACE:6:0:0
                }                                                                     ////__DACE:6:0:0
                {                                                                     ////__DACE:6:0:14
                    float y1 = ____y1[((4096 * i) + j)];                                              ////__DACE:6:0:9,14    ////__DACE:6:0:14
                    float ym1;                                                                        ////__DACE:6:0:14    ////__DACE:6:0:14
                    ////__DACE:6:0:14                                                 ////__DACE:6:0:14
                    ///////////////////                                                               ////__DACE:6:0:14    ////__DACE:6:0:14
                    ym1=y1;                                                                           ////__DACE:6:0:14    ////__DACE:6:0:14
                    ///////////////////                                                               ////__DACE:6:0:14    ////__DACE:6:0:14
                    ////__DACE:6:0:14                                                 ////__DACE:6:0:14
                    ___ym1 = ym1;                                                                     ////__DACE:6:0:14    ////__DACE:6:0:14
                }                                                                     ////__DACE:6:0:14
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:6:0:10
                    float imgIn = ____imgIn[((4096 * i) + j)];                                        ////__DACE:6:0:2,10    ////__DACE:6:0:10
                    float xm1;                                                                        ////__DACE:6:0:10    ////__DACE:6:0:10
                    ////__DACE:6:0:10                                                 ////__DACE:6:0:10
                    ///////////////////                                                               ////__DACE:6:0:10    ////__DACE:6:0:10
                    xm1=imgIn;                                                                        ////__DACE:6:0:10    ////__DACE:6:0:10
                    ///////////////////                                                               ////__DACE:6:0:10    ////__DACE:6:0:10
                    ////__DACE:6:0:10                                                 ////__DACE:6:0:10
                    ___xm1 = xm1;                                                                     ////__DACE:6:0:10    ////__DACE:6:0:10
                }                                                                     ////__DACE:6:0:10
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:6:0:12
                    float ym1 = ___ym1;                                                               ////__DACE:6:0:6,12    ////__DACE:6:0:12
                    float ym2;                                                                        ////__DACE:6:0:12    ////__DACE:6:0:12
                    ////__DACE:6:0:12                                                 ////__DACE:6:0:12
                    ///////////////////                                                               ////__DACE:6:0:12    ////__DACE:6:0:12
                    ym2=ym1;                                                                          ////__DACE:6:0:12    ////__DACE:6:0:12
                    ///////////////////                                                               ////__DACE:6:0:12    ////__DACE:6:0:12
                    ////__DACE:6:0:12                                                 ////__DACE:6:0:12
                    ___ym2 = ym2;                                                                     ////__DACE:6:0:12    ////__DACE:6:0:12
                }                                                                     ////__DACE:6:0:12
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:6:0
    
}                                                                                 ////__DACE:5:3:0
////__DACE:5:3:0
void FOR83_4_2_0(int& ___w, int& ___h, float* ___imgIn, float* ___y1, float& __xm1, float& __ym1, float& __ym2, float& __a1, float& __a2, float& __b1, float& __b2, int _argcount, int i) {    ////__DACE:4:2:0
    long long j;                                                                      ////__DACE:5
    ////__DACE:5
    {                                                                                 ////__DACE:5:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:5:0:0
                    float ym1;                                                                        ////__DACE:5:0:0    ////__DACE:5:0:0
                    ////__DACE:5:0:0                                                  ////__DACE:5:0:0
                    ///////////////////                                                               ////__DACE:5:0:0    ////__DACE:5:0:0
                    ym1=0.0f;                                                                         ////__DACE:5:0:0    ////__DACE:5:0:0
                    ///////////////////                                                               ////__DACE:5:0:0    ////__DACE:5:0:0
                    ////__DACE:5:0:0                                                  ////__DACE:5:0:0
                    __ym1 = ym1;                                                                      ////__DACE:5:0:0    ////__DACE:5:0:0
                }                                                                     ////__DACE:5:0:0
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:5:0:2
                    float ym2;                                                                        ////__DACE:5:0:2    ////__DACE:5:0:2
                    ////__DACE:5:0:2                                                  ////__DACE:5:0:2
                    ///////////////////                                                               ////__DACE:5:0:2    ////__DACE:5:0:2
                    ym2=0.0f;                                                                         ////__DACE:5:0:2    ////__DACE:5:0:2
                    ///////////////////                                                               ////__DACE:5:0:2    ////__DACE:5:0:2
                    ////__DACE:5:0:2                                                  ////__DACE:5:0:2
                    __ym2 = ym2;                                                                      ////__DACE:5:0:2    ////__DACE:5:0:2
                }                                                                     ////__DACE:5:0:2
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:5:0:4
                    float xm1;                                                                        ////__DACE:5:0:4    ////__DACE:5:0:4
                    ////__DACE:5:0:4                                                  ////__DACE:5:0:4
                    ///////////////////                                                               ////__DACE:5:0:4    ////__DACE:5:0:4
                    xm1=0.0f;                                                                         ////__DACE:5:0:4    ////__DACE:5:0:4
                    ///////////////////                                                               ////__DACE:5:0:4    ////__DACE:5:0:4
                    ////__DACE:5:0:4                                                  ////__DACE:5:0:4
                    __xm1 = xm1;                                                                      ////__DACE:5:0:4    ////__DACE:5:0:4
                }                                                                     ////__DACE:5:0:4
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:5:0
    for (j = 0; (j < ___h); j = j+1) {                                                ////__DACE:5:2
        {                                                                             ////__DACE:5:3
            
            
            FOR87_5_3_0(___h, &___imgIn[0], &___y1[0], __xm1, __ym1, __ym2, __a1, __a2, __b1, __b2, _argcount, i, j);    ////__DACE:5:3:0
        }                                                                             ////__DACE:5:3
    }                                                                                 ////__DACE:5:2
    
}                                                                                 ////__DACE:4:2:0
////__DACE:4:2:0
void FOR100_7_2_0(int& ____h, float* ____imgIn, float* ____y2, float& ___xp1, float& ___xp2, float& ___yp1, float& ___yp2, float& ___a3, float& ___a4, float& ___b1, float& ___b2, int _argcount, int i, int j) {    ////__DACE:7:2:0
    ////__DACE:8
    {                                                                                 ////__DACE:8:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:8:0:0
                    float a3 = ___a3;                                                                 ////__DACE:8:0:1,0    ////__DACE:8:0:0
                    float xp1 = ___xp1;                                                               ////__DACE:8:0:2,0    ////__DACE:8:0:0
                    float a4 = ___a4;                                                                 ////__DACE:8:0:3,0    ////__DACE:8:0:0
                    float xp2 = ___xp2;                                                               ////__DACE:8:0:4,0    ////__DACE:8:0:0
                    float b1 = ___b1;                                                                 ////__DACE:8:0:5,0    ////__DACE:8:0:0
                    float yp1 = ___yp1;                                                               ////__DACE:8:0:6,0    ////__DACE:8:0:0
                    float b2 = ___b2;                                                                 ////__DACE:8:0:7,0    ////__DACE:8:0:0
                    float yp2 = ___yp2;                                                               ////__DACE:8:0:8,0    ////__DACE:8:0:0
                    float y2;                                                                         ////__DACE:8:0:0    ////__DACE:8:0:0
                    ////__DACE:8:0:0                                                  ////__DACE:8:0:0
                    ///////////////////                                                               ////__DACE:8:0:0    ////__DACE:8:0:0
                    y2=a3*xp1+a4*xp2+b1*yp1+b2*yp2;                                                   ////__DACE:8:0:0    ////__DACE:8:0:0
                    ///////////////////                                                               ////__DACE:8:0:0    ////__DACE:8:0:0
                    ////__DACE:8:0:0                                                  ////__DACE:8:0:0
                    ____y2[((4096 * i) + j)] = y2;                                                    ////__DACE:8:0:0    ////__DACE:8:0:0
                }                                                                     ////__DACE:8:0:0
                {                                                                     ////__DACE:8:0:17
                    float y2 = ____y2[((4096 * i) + j)];                                              ////__DACE:8:0:9,17    ////__DACE:8:0:17
                    float yp1;                                                                        ////__DACE:8:0:17    ////__DACE:8:0:17
                    ////__DACE:8:0:17                                                 ////__DACE:8:0:17
                    ///////////////////                                                               ////__DACE:8:0:17    ////__DACE:8:0:17
                    yp1=y2;                                                                           ////__DACE:8:0:17    ////__DACE:8:0:17
                    ///////////////////                                                               ////__DACE:8:0:17    ////__DACE:8:0:17
                    ////__DACE:8:0:17                                                 ////__DACE:8:0:17
                    ___yp1 = yp1;                                                                     ////__DACE:8:0:17    ////__DACE:8:0:17
                }                                                                     ////__DACE:8:0:17
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:8:0:10
                    float xp1 = ___xp1;                                                               ////__DACE:8:0:2,10    ////__DACE:8:0:10
                    float xp2;                                                                        ////__DACE:8:0:10    ////__DACE:8:0:10
                    ////__DACE:8:0:10                                                 ////__DACE:8:0:10
                    ///////////////////                                                               ////__DACE:8:0:10    ////__DACE:8:0:10
                    xp2=xp1;                                                                          ////__DACE:8:0:10    ////__DACE:8:0:10
                    ///////////////////                                                               ////__DACE:8:0:10    ////__DACE:8:0:10
                    ////__DACE:8:0:10                                                 ////__DACE:8:0:10
                    ___xp2 = xp2;                                                                     ////__DACE:8:0:10    ////__DACE:8:0:10
                }                                                                     ////__DACE:8:0:10
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:8:0:15
                    float yp1 = ___yp1;                                                               ////__DACE:8:0:6,15    ////__DACE:8:0:15
                    float yp2;                                                                        ////__DACE:8:0:15    ////__DACE:8:0:15
                    ////__DACE:8:0:15                                                 ////__DACE:8:0:15
                    ///////////////////                                                               ////__DACE:8:0:15    ////__DACE:8:0:15
                    yp2=yp1;                                                                          ////__DACE:8:0:15    ////__DACE:8:0:15
                    ///////////////////                                                               ////__DACE:8:0:15    ////__DACE:8:0:15
                    ////__DACE:8:0:15                                                 ////__DACE:8:0:15
                    ___yp2 = yp2;                                                                     ////__DACE:8:0:15    ////__DACE:8:0:15
                }                                                                     ////__DACE:8:0:15
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:8:0:12
                    float imgIn = ____imgIn[((4096 * i) + j)];                                        ////__DACE:8:0:13,12    ////__DACE:8:0:12
                    float xp1;                                                                        ////__DACE:8:0:12    ////__DACE:8:0:12
                    ////__DACE:8:0:12                                                 ////__DACE:8:0:12
                    ///////////////////                                                               ////__DACE:8:0:12    ////__DACE:8:0:12
                    xp1=imgIn;                                                                        ////__DACE:8:0:12    ////__DACE:8:0:12
                    ///////////////////                                                               ////__DACE:8:0:12    ////__DACE:8:0:12
                    ////__DACE:8:0:12                                                 ////__DACE:8:0:12
                    ___xp1 = xp1;                                                                     ////__DACE:8:0:12    ////__DACE:8:0:12
                }                                                                     ////__DACE:8:0:12
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:8:0
    
}                                                                                 ////__DACE:7:2:0
////__DACE:7:2:0
void FOR95_4_4_0(int& ___w, int& ___h, float* ___imgIn, float* ___y2, float& __xp1, float& __xp2, float& __yp1, float& __yp2, float& __a3, float& __a4, float& __b1, float& __b2, int _argcount, int i) {    ////__DACE:4:4:0
    long long j;                                                                      ////__DACE:7
    ////__DACE:7
    __state_7_state96:;                                                               ////__DACE:7:0
    {                                                                                 ////__DACE:7:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:7:0:0
                    float yp1;                                                                        ////__DACE:7:0:0    ////__DACE:7:0:0
                    ////__DACE:7:0:0                                                  ////__DACE:7:0:0
                    ///////////////////                                                               ////__DACE:7:0:0    ////__DACE:7:0:0
                    yp1=0.0f;                                                                         ////__DACE:7:0:0    ////__DACE:7:0:0
                    ///////////////////                                                               ////__DACE:7:0:0    ////__DACE:7:0:0
                    ////__DACE:7:0:0                                                  ////__DACE:7:0:0
                    __yp1 = yp1;                                                                      ////__DACE:7:0:0    ////__DACE:7:0:0
                }                                                                     ////__DACE:7:0:0
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:7:0:2
                    float yp2;                                                                        ////__DACE:7:0:2    ////__DACE:7:0:2
                    ////__DACE:7:0:2                                                  ////__DACE:7:0:2
                    ///////////////////                                                               ////__DACE:7:0:2    ////__DACE:7:0:2
                    yp2=0.0f;                                                                         ////__DACE:7:0:2    ////__DACE:7:0:2
                    ///////////////////                                                               ////__DACE:7:0:2    ////__DACE:7:0:2
                    ////__DACE:7:0:2                                                  ////__DACE:7:0:2
                    __yp2 = yp2;                                                                      ////__DACE:7:0:2    ////__DACE:7:0:2
                }                                                                     ////__DACE:7:0:2
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:7:0:4
                    float xp1;                                                                        ////__DACE:7:0:4    ////__DACE:7:0:4
                    ////__DACE:7:0:4                                                  ////__DACE:7:0:4
                    ///////////////////                                                               ////__DACE:7:0:4    ////__DACE:7:0:4
                    xp1=0.0f;                                                                         ////__DACE:7:0:4    ////__DACE:7:0:4
                    ///////////////////                                                               ////__DACE:7:0:4    ////__DACE:7:0:4
                    ////__DACE:7:0:4                                                  ////__DACE:7:0:4
                    __xp1 = xp1;                                                                      ////__DACE:7:0:4    ////__DACE:7:0:4
                }                                                                     ////__DACE:7:0:4
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:7:0:6
                    float xp2;                                                                        ////__DACE:7:0:6    ////__DACE:7:0:6
                    ////__DACE:7:0:6                                                  ////__DACE:7:0:6
                    ///////////////////                                                               ////__DACE:7:0:6    ////__DACE:7:0:6
                    xp2=0.0f;                                                                         ////__DACE:7:0:6    ////__DACE:7:0:6
                    ///////////////////                                                               ////__DACE:7:0:6    ////__DACE:7:0:6
                    ////__DACE:7:0:6                                                  ////__DACE:7:0:6
                    __xp2 = xp2;                                                                      ////__DACE:7:0:6    ////__DACE:7:0:6
                }                                                                     ////__DACE:7:0:6
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:7:0
    if ((j >= 0)) {                                                                   ////__DACE:7:0
        goto __state_7_stateFOR100;                                                   ////__DACE:7:0
    }
    if (((j >= 0) == false)) {                                                        ////__DACE:7:0
        goto __state_7_MergeState100;                                                 ////__DACE:7:0
    }
    __state_7_stateFOR100:;                                                           ////__DACE:7:2
    {                                                                                 ////__DACE:7:2
        
        
        FOR100_7_2_0(___h, &___imgIn[0], &___y2[0], __xp1, __xp2, __yp1, __yp2, __a3, __a4, __b1, __b2, _argcount, i, j);    ////__DACE:7:2:0
    }                                                                                 ////__DACE:7:2
    j = j-1;                                                                          ////__DACE:7:2
    goto __state_7_state96;                                                           ////__DACE:7:2
    __state_7_MergeState100:;                                                         ////__DACE:7:1
    
}                                                                                 ////__DACE:4:4:0
////__DACE:4:4:0
void FOR118_9_3_0(int& ____w, float* ____imgOut, float* ____y1, float& ___tm1, float& ___ym1, float& ___ym2, float& ___a5, float& ___a6, float& ___b1, float& ___b2, int _argcount, int i, int j) {    ////__DACE:9:3:0
    ////__DACE:10
    {                                                                                 ////__DACE:10:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:10:0:0
                    float a5 = ___a5;                                                                 ////__DACE:10:0:1,0    ////__DACE:10:0:0
                    float imgOut = ____imgOut[((4096 * i) + j)];                                      ////__DACE:10:0:2,0    ////__DACE:10:0:0
                    float a6 = ___a6;                                                                 ////__DACE:10:0:3,0    ////__DACE:10:0:0
                    float tm1 = ___tm1;                                                               ////__DACE:10:0:4,0    ////__DACE:10:0:0
                    float b1 = ___b1;                                                                 ////__DACE:10:0:5,0    ////__DACE:10:0:0
                    float ym1 = ___ym1;                                                               ////__DACE:10:0:6,0    ////__DACE:10:0:0
                    float b2 = ___b2;                                                                 ////__DACE:10:0:7,0    ////__DACE:10:0:0
                    float ym2 = ___ym2;                                                               ////__DACE:10:0:8,0    ////__DACE:10:0:0
                    float y1;                                                                         ////__DACE:10:0:0    ////__DACE:10:0:0
                    ////__DACE:10:0:0                                                 ////__DACE:10:0:0
                    ///////////////////                                                               ////__DACE:10:0:0    ////__DACE:10:0:0
                    y1=a5*imgOut+a6*tm1+b1*ym1+b2*ym2;                                                ////__DACE:10:0:0    ////__DACE:10:0:0
                    ///////////////////                                                               ////__DACE:10:0:0    ////__DACE:10:0:0
                    ////__DACE:10:0:0                                                 ////__DACE:10:0:0
                    ____y1[((4096 * i) + j)] = y1;                                                    ////__DACE:10:0:0    ////__DACE:10:0:0
                }                                                                     ////__DACE:10:0:0
                {                                                                     ////__DACE:10:0:14
                    float y1 = ____y1[((4096 * i) + j)];                                              ////__DACE:10:0:9,14    ////__DACE:10:0:14
                    float ym1;                                                                        ////__DACE:10:0:14    ////__DACE:10:0:14
                    ////__DACE:10:0:14                                                ////__DACE:10:0:14
                    ///////////////////                                                               ////__DACE:10:0:14    ////__DACE:10:0:14
                    ym1=y1;                                                                           ////__DACE:10:0:14    ////__DACE:10:0:14
                    ///////////////////                                                               ////__DACE:10:0:14    ////__DACE:10:0:14
                    ////__DACE:10:0:14                                                ////__DACE:10:0:14
                    ___ym1 = ym1;                                                                     ////__DACE:10:0:14    ////__DACE:10:0:14
                }                                                                     ////__DACE:10:0:14
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:10:0:10
                    float imgOut = ____imgOut[((4096 * i) + j)];                                      ////__DACE:10:0:2,10    ////__DACE:10:0:10
                    float tm1;                                                                        ////__DACE:10:0:10    ////__DACE:10:0:10
                    ////__DACE:10:0:10                                                ////__DACE:10:0:10
                    ///////////////////                                                               ////__DACE:10:0:10    ////__DACE:10:0:10
                    tm1=imgOut;                                                                       ////__DACE:10:0:10    ////__DACE:10:0:10
                    ///////////////////                                                               ////__DACE:10:0:10    ////__DACE:10:0:10
                    ////__DACE:10:0:10                                                ////__DACE:10:0:10
                    ___tm1 = tm1;                                                                     ////__DACE:10:0:10    ////__DACE:10:0:10
                }                                                                     ////__DACE:10:0:10
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:10:0:12
                    float ym1 = ___ym1;                                                               ////__DACE:10:0:6,12    ////__DACE:10:0:12
                    float ym2;                                                                        ////__DACE:10:0:12    ////__DACE:10:0:12
                    ////__DACE:10:0:12                                                ////__DACE:10:0:12
                    ///////////////////                                                               ////__DACE:10:0:12    ////__DACE:10:0:12
                    ym2=ym1;                                                                          ////__DACE:10:0:12    ////__DACE:10:0:12
                    ///////////////////                                                               ////__DACE:10:0:12    ////__DACE:10:0:12
                    ////__DACE:10:0:12                                                ////__DACE:10:0:12
                    ___ym2 = ym2;                                                                     ////__DACE:10:0:12    ////__DACE:10:0:12
                }                                                                     ////__DACE:10:0:12
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:10:0
    
}                                                                                 ////__DACE:9:3:0
////__DACE:9:3:0
void FOR114_4_6_0(int& ___w, int& ___h, float* ___imgOut, float* ___y1, float& __tm1, float& __ym1, float& __ym2, float& __a5, float& __a6, float& __b1, float& __b2, int _argcount, int i, int j) {    ////__DACE:4:6:0
    long long i;                                                                      ////__DACE:9
    ////__DACE:9
    {                                                                                 ////__DACE:9:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:9:0:0
                    float tm1;                                                                        ////__DACE:9:0:0    ////__DACE:9:0:0
                    ////__DACE:9:0:0                                                  ////__DACE:9:0:0
                    ///////////////////                                                               ////__DACE:9:0:0    ////__DACE:9:0:0
                    tm1=0.0f;                                                                         ////__DACE:9:0:0    ////__DACE:9:0:0
                    ///////////////////                                                               ////__DACE:9:0:0    ////__DACE:9:0:0
                    ////__DACE:9:0:0                                                  ////__DACE:9:0:0
                    __tm1 = tm1;                                                                      ////__DACE:9:0:0    ////__DACE:9:0:0
                }                                                                     ////__DACE:9:0:0
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:9:0:2
                    float ym1;                                                                        ////__DACE:9:0:2    ////__DACE:9:0:2
                    ////__DACE:9:0:2                                                  ////__DACE:9:0:2
                    ///////////////////                                                               ////__DACE:9:0:2    ////__DACE:9:0:2
                    ym1=0.0f;                                                                         ////__DACE:9:0:2    ////__DACE:9:0:2
                    ///////////////////                                                               ////__DACE:9:0:2    ////__DACE:9:0:2
                    ////__DACE:9:0:2                                                  ////__DACE:9:0:2
                    __ym1 = ym1;                                                                      ////__DACE:9:0:2    ////__DACE:9:0:2
                }                                                                     ////__DACE:9:0:2
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:9:0:4
                    float ym2;                                                                        ////__DACE:9:0:4    ////__DACE:9:0:4
                    ////__DACE:9:0:4                                                  ////__DACE:9:0:4
                    ///////////////////                                                               ////__DACE:9:0:4    ////__DACE:9:0:4
                    ym2=0.0f;                                                                         ////__DACE:9:0:4    ////__DACE:9:0:4
                    ///////////////////                                                               ////__DACE:9:0:4    ////__DACE:9:0:4
                    ////__DACE:9:0:4                                                  ////__DACE:9:0:4
                    __ym2 = ym2;                                                                      ////__DACE:9:0:4    ////__DACE:9:0:4
                }                                                                     ////__DACE:9:0:4
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:9:0
    for (i = 0; (i < ___w); i = i+1) {                                                ////__DACE:9:2
        {                                                                             ////__DACE:9:3
            
            
            FOR118_9_3_0(___w, &___imgOut[0], &___y1[0], __tm1, __ym1, __ym2, __a5, __a6, __b1, __b2, _argcount, i, j);    ////__DACE:9:3:0
        }                                                                             ////__DACE:9:3
    }                                                                                 ////__DACE:9:2
    
}                                                                                 ////__DACE:4:6:0
////__DACE:4:6:0
void FOR132_11_2_0(int& ____w, float* ____imgOut, float* ____y2, float& ___tp1, float& ___tp2, float& ___yp1, float& ___yp2, float& ___a7, float& ___a8, float& ___b1, float& ___b2, int _argcount, int i, int j) {    ////__DACE:11:2:0
    ////__DACE:12
    {                                                                                 ////__DACE:12:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:12:0:0
                    float a7 = ___a7;                                                                 ////__DACE:12:0:1,0    ////__DACE:12:0:0
                    float tp1 = ___tp1;                                                               ////__DACE:12:0:2,0    ////__DACE:12:0:0
                    float a8 = ___a8;                                                                 ////__DACE:12:0:3,0    ////__DACE:12:0:0
                    float tp2 = ___tp2;                                                               ////__DACE:12:0:4,0    ////__DACE:12:0:0
                    float b1 = ___b1;                                                                 ////__DACE:12:0:5,0    ////__DACE:12:0:0
                    float yp1 = ___yp1;                                                               ////__DACE:12:0:6,0    ////__DACE:12:0:0
                    float b2 = ___b2;                                                                 ////__DACE:12:0:7,0    ////__DACE:12:0:0
                    float yp2 = ___yp2;                                                               ////__DACE:12:0:8,0    ////__DACE:12:0:0
                    float y2;                                                                         ////__DACE:12:0:0    ////__DACE:12:0:0
                    ////__DACE:12:0:0                                                 ////__DACE:12:0:0
                    ///////////////////                                                               ////__DACE:12:0:0    ////__DACE:12:0:0
                    y2=a7*tp1+a8*tp2+b1*yp1+b2*yp2;                                                   ////__DACE:12:0:0    ////__DACE:12:0:0
                    ///////////////////                                                               ////__DACE:12:0:0    ////__DACE:12:0:0
                    ////__DACE:12:0:0                                                 ////__DACE:12:0:0
                    ____y2[((4096 * i) + j)] = y2;                                                    ////__DACE:12:0:0    ////__DACE:12:0:0
                }                                                                     ////__DACE:12:0:0
                {                                                                     ////__DACE:12:0:17
                    float y2 = ____y2[((4096 * i) + j)];                                              ////__DACE:12:0:9,17    ////__DACE:12:0:17
                    float yp1;                                                                        ////__DACE:12:0:17    ////__DACE:12:0:17
                    ////__DACE:12:0:17                                                ////__DACE:12:0:17
                    ///////////////////                                                               ////__DACE:12:0:17    ////__DACE:12:0:17
                    yp1=y2;                                                                           ////__DACE:12:0:17    ////__DACE:12:0:17
                    ///////////////////                                                               ////__DACE:12:0:17    ////__DACE:12:0:17
                    ////__DACE:12:0:17                                                ////__DACE:12:0:17
                    ___yp1 = yp1;                                                                     ////__DACE:12:0:17    ////__DACE:12:0:17
                }                                                                     ////__DACE:12:0:17
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:12:0:10
                    float tp1 = ___tp1;                                                               ////__DACE:12:0:2,10    ////__DACE:12:0:10
                    float tp2;                                                                        ////__DACE:12:0:10    ////__DACE:12:0:10
                    ////__DACE:12:0:10                                                ////__DACE:12:0:10
                    ///////////////////                                                               ////__DACE:12:0:10    ////__DACE:12:0:10
                    tp2=tp1;                                                                          ////__DACE:12:0:10    ////__DACE:12:0:10
                    ///////////////////                                                               ////__DACE:12:0:10    ////__DACE:12:0:10
                    ////__DACE:12:0:10                                                ////__DACE:12:0:10
                    ___tp2 = tp2;                                                                     ////__DACE:12:0:10    ////__DACE:12:0:10
                }                                                                     ////__DACE:12:0:10
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:12:0:15
                    float yp1 = ___yp1;                                                               ////__DACE:12:0:6,15    ////__DACE:12:0:15
                    float yp2;                                                                        ////__DACE:12:0:15    ////__DACE:12:0:15
                    ////__DACE:12:0:15                                                ////__DACE:12:0:15
                    ///////////////////                                                               ////__DACE:12:0:15    ////__DACE:12:0:15
                    yp2=yp1;                                                                          ////__DACE:12:0:15    ////__DACE:12:0:15
                    ///////////////////                                                               ////__DACE:12:0:15    ////__DACE:12:0:15
                    ////__DACE:12:0:15                                                ////__DACE:12:0:15
                    ___yp2 = yp2;                                                                     ////__DACE:12:0:15    ////__DACE:12:0:15
                }                                                                     ////__DACE:12:0:15
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:12:0:12
                    float imgOut = ____imgOut[((4096 * i) + j)];                                      ////__DACE:12:0:13,12    ////__DACE:12:0:12
                    float tp1;                                                                        ////__DACE:12:0:12    ////__DACE:12:0:12
                    ////__DACE:12:0:12                                                ////__DACE:12:0:12
                    ///////////////////                                                               ////__DACE:12:0:12    ////__DACE:12:0:12
                    tp1=imgOut;                                                                       ////__DACE:12:0:12    ////__DACE:12:0:12
                    ///////////////////                                                               ////__DACE:12:0:12    ////__DACE:12:0:12
                    ////__DACE:12:0:12                                                ////__DACE:12:0:12
                    ___tp1 = tp1;                                                                     ////__DACE:12:0:12    ////__DACE:12:0:12
                }                                                                     ////__DACE:12:0:12
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:12:0
    
}                                                                                 ////__DACE:11:2:0
////__DACE:11:2:0
void FOR127_4_8_0(int& ___w, int& ___h, float* ___imgOut, float* ___y2, float& __tp1, float& __tp2, float& __yp1, float& __yp2, float& __a7, float& __a8, float& __b1, float& __b2, int _argcount, int i, int j) {    ////__DACE:4:8:0
    long long i;                                                                      ////__DACE:11
    ////__DACE:11
    __state_11_state128:;                                                             ////__DACE:11:0
    {                                                                                 ////__DACE:11:0
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:11:0:0
                    float tp1;                                                                        ////__DACE:11:0:0    ////__DACE:11:0:0
                    ////__DACE:11:0:0                                                 ////__DACE:11:0:0
                    ///////////////////                                                               ////__DACE:11:0:0    ////__DACE:11:0:0
                    tp1=0.0f;                                                                         ////__DACE:11:0:0    ////__DACE:11:0:0
                    ///////////////////                                                               ////__DACE:11:0:0    ////__DACE:11:0:0
                    ////__DACE:11:0:0                                                 ////__DACE:11:0:0
                    __tp1 = tp1;                                                                      ////__DACE:11:0:0    ////__DACE:11:0:0
                }                                                                     ////__DACE:11:0:0
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:11:0:2
                    float tp2;                                                                        ////__DACE:11:0:2    ////__DACE:11:0:2
                    ////__DACE:11:0:2                                                 ////__DACE:11:0:2
                    ///////////////////                                                               ////__DACE:11:0:2    ////__DACE:11:0:2
                    tp2=0.0f;                                                                         ////__DACE:11:0:2    ////__DACE:11:0:2
                    ///////////////////                                                               ////__DACE:11:0:2    ////__DACE:11:0:2
                    ////__DACE:11:0:2                                                 ////__DACE:11:0:2
                    __tp2 = tp2;                                                                      ////__DACE:11:0:2    ////__DACE:11:0:2
                }                                                                     ////__DACE:11:0:2
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:11:0:4
                    float yp1;                                                                        ////__DACE:11:0:4    ////__DACE:11:0:4
                    ////__DACE:11:0:4                                                 ////__DACE:11:0:4
                    ///////////////////                                                               ////__DACE:11:0:4    ////__DACE:11:0:4
                    yp1=0.0f;                                                                         ////__DACE:11:0:4    ////__DACE:11:0:4
                    ///////////////////                                                               ////__DACE:11:0:4    ////__DACE:11:0:4
                    ////__DACE:11:0:4                                                 ////__DACE:11:0:4
                    __yp1 = yp1;                                                                      ////__DACE:11:0:4    ////__DACE:11:0:4
                }                                                                     ////__DACE:11:0:4
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:11:0:6
                    float yp2;                                                                        ////__DACE:11:0:6    ////__DACE:11:0:6
                    ////__DACE:11:0:6                                                 ////__DACE:11:0:6
                    ///////////////////                                                               ////__DACE:11:0:6    ////__DACE:11:0:6
                    yp2=0.0f;                                                                         ////__DACE:11:0:6    ////__DACE:11:0:6
                    ///////////////////                                                               ////__DACE:11:0:6    ////__DACE:11:0:6
                    ////__DACE:11:0:6                                                 ////__DACE:11:0:6
                    __yp2 = yp2;                                                                      ////__DACE:11:0:6    ////__DACE:11:0:6
                }                                                                     ////__DACE:11:0:6
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:11:0
    if ((i >= 0)) {                                                                   ////__DACE:11:0
        goto __state_11_stateFOR132;                                                  ////__DACE:11:0
    }
    if (((i >= 0) == false)) {                                                        ////__DACE:11:0
        goto __state_11_MergeState132;                                                ////__DACE:11:0
    }
    __state_11_stateFOR132:;                                                          ////__DACE:11:2
    {                                                                                 ////__DACE:11:2
        
        
        FOR132_11_2_0(___w, &___imgOut[0], &___y2[0], __tp1, __tp2, __yp1, __yp2, __a7, __a8, __b1, __b2, _argcount, i, j);    ////__DACE:11:2:0
    }                                                                                 ////__DACE:11:2
    i = i-1;                                                                          ////__DACE:11:2
    goto __state_11_state128;                                                         ////__DACE:11:2
    __state_11_MergeState132:;                                                        ////__DACE:11:1
    
}                                                                                 ////__DACE:4:8:0
////__DACE:4:8:0
void FOR141_4_11_0(int& ___w, int& ___h, float* ___imgOut, float* ___y1, float* ___y2, float& __c2, int _argcount, int i, int j) {    ////__DACE:4:11:0
    long long j;                                                                      ////__DACE:13
    ////__DACE:13
    for (j = 0; (j < ___h); j = j+1) {                                                ////__DACE:13:2
        {                                                                             ////__DACE:13:3
            
            
            {                                                                         ////__DACE:13:3:4
                float c2 = __c2;                                                                  ////__DACE:13:3:3,4    ////__DACE:13:3:4
                float y1 = ___y1[((4096 * i) + j)];                                               ////__DACE:13:3:1,4    ////__DACE:13:3:4
                float y2 = ___y2[((4096 * i) + j)];                                               ////__DACE:13:3:2,4    ////__DACE:13:3:4
                float imgOut;                                                                     ////__DACE:13:3:4    ////__DACE:13:3:4
                ////__DACE:13:3:4                                                     ////__DACE:13:3:4
                ///////////////////                                                               ////__DACE:13:3:4    ////__DACE:13:3:4
                imgOut=c2*(y1+y2);                                                                ////__DACE:13:3:4    ////__DACE:13:3:4
                ///////////////////                                                               ////__DACE:13:3:4    ////__DACE:13:3:4
                ////__DACE:13:3:4                                                     ////__DACE:13:3:4
                ___imgOut[((4096 * i) + j)] = imgOut;                                             ////__DACE:13:3:4    ////__DACE:13:3:4
            }                                                                         ////__DACE:13:3:4
        }                                                                             ////__DACE:13:3
    }                                                                                 ////__DACE:13:2
    
}                                                                                 ////__DACE:4:11:0
////__DACE:4:11:0
void kernel_deriche_1_0_11(int& __w, int& __h, float& __alpha, float* __imgIn, float* __imgOut, float* __y1, float* __y2, int _argcount) {    ////__DACE:1:0:11
    float _a1;                                                                        ////__DACE:4
    float _a5;                                                                        ////__DACE:4
    float _a2;                                                                        ////__DACE:4
    float _a6;                                                                        ////__DACE:4
    float _a3;                                                                        ////__DACE:4
    float _a7;                                                                        ////__DACE:4
    float _a4;                                                                        ////__DACE:4
    float _a8;                                                                        ////__DACE:4
    float _b1;                                                                        ////__DACE:4
    float _b2;                                                                        ////__DACE:4
    float _c2;                                                                        ////__DACE:4
    float _ym1;                                                                       ////__DACE:4
    float _ym2;                                                                       ////__DACE:4
    float _yp1;                                                                       ////__DACE:4
    float _yp2;                                                                       ////__DACE:4
    long long i;                                                                      ////__DACE:4
    long long j;                                                                      ////__DACE:4
    ////__DACE:4
    {                                                                                 ////__DACE:4:0
        float _k;                                                                     ////__DACE:4:0:2
        float _c1;                                                                    ////__DACE:4:0:20
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:4:0:0
                    float alpha = __alpha;                                                            ////__DACE:4:0:1,0    ////__DACE:4:0:0
                    float k;                                                                          ////__DACE:4:0:0    ////__DACE:4:0:0
                    ////__DACE:4:0:0                                                  ////__DACE:4:0:0
                    ///////////////////                                                               ////__DACE:4:0:0    ////__DACE:4:0:0
                    k=(1.0f-expf(-alpha))*(1.0f-expf(-alpha))/(1.0f+2.0f*alpha*expf(-alpha)-expf(2.0f*alpha));    ////__DACE:4:0:0    ////__DACE:4:0:0
                    ///////////////////                                                               ////__DACE:4:0:0    ////__DACE:4:0:0
                    ////__DACE:4:0:0                                                  ////__DACE:4:0:0
                    _k = k;                                                                           ////__DACE:4:0:0    ////__DACE:4:0:0
                }                                                                     ////__DACE:4:0:0
                {                                                                     ////__DACE:4:0:3
                    float k = _k;                                                                     ////__DACE:4:0:2,3    ////__DACE:4:0:3
                    float a1;                                                                         ////__DACE:4:0:3    ////__DACE:4:0:3
                    float a5;                                                                         ////__DACE:4:0:3    ////__DACE:4:0:3
                    ////__DACE:4:0:3                                                  ////__DACE:4:0:3
                    ///////////////////                                                               ////__DACE:4:0:3    ////__DACE:4:0:3
                    a1=a5=k;                                                                          ////__DACE:4:0:3    ////__DACE:4:0:3
                    ///////////////////                                                               ////__DACE:4:0:3    ////__DACE:4:0:3
                    ////__DACE:4:0:3                                                  ////__DACE:4:0:3
                    _a1 = a1;                                                                         ////__DACE:4:0:3    ////__DACE:4:0:3
                    _a5 = a5;                                                                         ////__DACE:4:0:3    ////__DACE:4:0:3
                }                                                                     ////__DACE:4:0:3
                {                                                                     ////__DACE:4:0:6
                    float k = _k;                                                                     ////__DACE:4:0:2,6    ////__DACE:4:0:6
                    float alpha = __alpha;                                                            ////__DACE:4:0:1,6    ////__DACE:4:0:6
                    float a2;                                                                         ////__DACE:4:0:6    ////__DACE:4:0:6
                    float a6;                                                                         ////__DACE:4:0:6    ////__DACE:4:0:6
                    ////__DACE:4:0:6                                                  ////__DACE:4:0:6
                    ///////////////////                                                               ////__DACE:4:0:6    ////__DACE:4:0:6
                    a2=a6=k*expf(-alpha)*(alpha-1.0f);                                                ////__DACE:4:0:6    ////__DACE:4:0:6
                    ///////////////////                                                               ////__DACE:4:0:6    ////__DACE:4:0:6
                    ////__DACE:4:0:6                                                  ////__DACE:4:0:6
                    _a2 = a2;                                                                         ////__DACE:4:0:6    ////__DACE:4:0:6
                    _a6 = a6;                                                                         ////__DACE:4:0:6    ////__DACE:4:0:6
                }                                                                     ////__DACE:4:0:6
                {                                                                     ////__DACE:4:0:9
                    float k = _k;                                                                     ////__DACE:4:0:2,9    ////__DACE:4:0:9
                    float alpha = __alpha;                                                            ////__DACE:4:0:1,9    ////__DACE:4:0:9
                    float a3;                                                                         ////__DACE:4:0:9    ////__DACE:4:0:9
                    float a7;                                                                         ////__DACE:4:0:9    ////__DACE:4:0:9
                    ////__DACE:4:0:9                                                  ////__DACE:4:0:9
                    ///////////////////                                                               ////__DACE:4:0:9    ////__DACE:4:0:9
                    a3=a7=k*expf(-alpha)*(alpha+1.0f);                                                ////__DACE:4:0:9    ////__DACE:4:0:9
                    ///////////////////                                                               ////__DACE:4:0:9    ////__DACE:4:0:9
                    ////__DACE:4:0:9                                                  ////__DACE:4:0:9
                    _a3 = a3;                                                                         ////__DACE:4:0:9    ////__DACE:4:0:9
                    _a7 = a7;                                                                         ////__DACE:4:0:9    ////__DACE:4:0:9
                }                                                                     ////__DACE:4:0:9
                {                                                                     ////__DACE:4:0:12
                    float k = _k;                                                                     ////__DACE:4:0:2,12    ////__DACE:4:0:12
                    float a4;                                                                         ////__DACE:4:0:12    ////__DACE:4:0:12
                    float a8;                                                                         ////__DACE:4:0:12    ////__DACE:4:0:12
                    ////__DACE:4:0:12                                                 ////__DACE:4:0:12
                    ///////////////////                                                               ////__DACE:4:0:12    ////__DACE:4:0:12
                    a4=a8=-k*expf(-2.0f*alpha);                                                       ////__DACE:4:0:12    ////__DACE:4:0:12
                    ///////////////////                                                               ////__DACE:4:0:12    ////__DACE:4:0:12
                    ////__DACE:4:0:12                                                 ////__DACE:4:0:12
                    _a4 = a4;                                                                         ////__DACE:4:0:12    ////__DACE:4:0:12
                    _a8 = a8;                                                                         ////__DACE:4:0:12    ////__DACE:4:0:12
                }                                                                     ////__DACE:4:0:12
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:4:0:15
                    float b1;                                                                         ////__DACE:4:0:15    ////__DACE:4:0:15
                    ////__DACE:4:0:15                                                 ////__DACE:4:0:15
                    ///////////////////                                                               ////__DACE:4:0:15    ////__DACE:4:0:15
                    b1=powf(2.0f,-alpha);                                                             ////__DACE:4:0:15    ////__DACE:4:0:15
                    ///////////////////                                                               ////__DACE:4:0:15    ////__DACE:4:0:15
                    ////__DACE:4:0:15                                                 ////__DACE:4:0:15
                    _b1 = b1;                                                                         ////__DACE:4:0:15    ////__DACE:4:0:15
                }                                                                     ////__DACE:4:0:15
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:4:0:17
                    float b2;                                                                         ////__DACE:4:0:17    ////__DACE:4:0:17
                    ////__DACE:4:0:17                                                 ////__DACE:4:0:17
                    ///////////////////                                                               ////__DACE:4:0:17    ////__DACE:4:0:17
                    b2=-expf(-2.0f*alpha);                                                            ////__DACE:4:0:17    ////__DACE:4:0:17
                    ///////////////////                                                               ////__DACE:4:0:17    ////__DACE:4:0:17
                    ////__DACE:4:0:17                                                 ////__DACE:4:0:17
                    _b2 = b2;                                                                         ////__DACE:4:0:17    ////__DACE:4:0:17
                }                                                                     ////__DACE:4:0:17
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:4:0:19
                    float c1;                                                                         ////__DACE:4:0:19    ////__DACE:4:0:19
                    float c2;                                                                         ////__DACE:4:0:19    ////__DACE:4:0:19
                    ////__DACE:4:0:19                                                 ////__DACE:4:0:19
                    ///////////////////                                                               ////__DACE:4:0:19    ////__DACE:4:0:19
                    c1=c2=1;                                                                          ////__DACE:4:0:19    ////__DACE:4:0:19
                    ///////////////////                                                               ////__DACE:4:0:19    ////__DACE:4:0:19
                    ////__DACE:4:0:19                                                 ////__DACE:4:0:19
                    _c1 = c1;                                                                         ////__DACE:4:0:19    ////__DACE:4:0:19
                    _c2 = c2;                                                                         ////__DACE:4:0:19    ////__DACE:4:0:19
                }                                                                     ////__DACE:4:0:19
            } // End omp section
        } // End omp sections
    }                                                                                 ////__DACE:4:0
    for (i = 0; (i < __w); i = i+1) {                                                 ////__DACE:4:1
        {                                                                             ////__DACE:4:2
            float _xm1;                                                               ////__DACE:4:2:9
            
            
            FOR83_4_2_0(__w, __h, &__imgIn[0], &__y1[0], _xm1, _ym1, _ym2, _a1, _a2, _b1, _b2, _argcount, i);    ////__DACE:4:2:0
        }                                                                             ////__DACE:4:2
    }                                                                                 ////__DACE:4:1
    i = 0;                                                                            ////__DACE:4:1
    for (; (i < __w); i = i+1) {                                                      ////__DACE:4:3
        {                                                                             ////__DACE:4:4
            float _xp1;                                                               ////__DACE:4:4:9
            float _xp2;                                                               ////__DACE:4:4:11
            
            
            FOR95_4_4_0(__w, __h, &__imgIn[0], &__y2[0], _xp1, _xp2, _yp1, _yp2, _a3, _a4, _b1, _b2, _argcount, i);    ////__DACE:4:4:0
        }                                                                             ////__DACE:4:4
    }                                                                                 ////__DACE:4:3
    j = 0;                                                                            ////__DACE:4:3
    for (; (j < __h); j = j+1) {                                                      ////__DACE:4:5
        {                                                                             ////__DACE:4:6
            float _tm1;                                                               ////__DACE:4:6:9
            
            
            FOR114_4_6_0(__w, __h, &__imgOut[0], &__y1[0], _tm1, _ym1, _ym2, _a5, _a6, _b1, _b2, _argcount, i, j);    ////__DACE:4:6:0
        }                                                                             ////__DACE:4:6
    }                                                                                 ////__DACE:4:5
    j = 0;                                                                            ////__DACE:4:5
    for (; (j < __h); j = j+1) {                                                      ////__DACE:4:7
        {                                                                             ////__DACE:4:8
            float _tp1;                                                               ////__DACE:4:8:9
            float _tp2;                                                               ////__DACE:4:8:11
            
            
            FOR127_4_8_0(__w, __h, &__imgOut[0], &__y2[0], _tp1, _tp2, _yp1, _yp2, _a7, _a8, _b1, _b2, _argcount, i, j);    ////__DACE:4:8:0
        }                                                                             ////__DACE:4:8
    }                                                                                 ////__DACE:4:7
    i = 0;                                                                            ////__DACE:4:7
    for (; (i < __w); i = i+1) {                                                      ////__DACE:4:10
        {                                                                             ////__DACE:4:11
            
            
            FOR141_4_11_0(__w, __h, &__imgOut[0], &__y1[0], &__y2[0], _c2, _argcount, i, j);    ////__DACE:4:11:0
        }                                                                             ////__DACE:4:11
    }                                                                                 ////__DACE:4:10
    
}                                                                                 ////__DACE:1:0:11
////__DACE:1:0:11
void main_0_0_0(int& __argc, double* __argv, int _argcount) {                     ////__DACE:0:0:0
    ////__DACE:1
    {                                                                                 ////__DACE:1:0
        int _w;                                                                       ////__DACE:1:0:1
        int _h;                                                                       ////__DACE:1:0:3
        float _alpha;                                                                 ////__DACE:1:0:7
        float *_imgIn = new float DACE_ALIGN(64)[8847360];                            ////__DACE:1:0:9
        float *_imgOut = new float DACE_ALIGN(64)[8847360];                           ////__DACE:1:0:16
        float *_y1 = new float DACE_ALIGN(64)[8847360];                               ////__DACE:1:0:18
        float *_y2 = new float DACE_ALIGN(64)[8847360];                               ////__DACE:1:0:20
        
        
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                {                                                                     ////__DACE:1:0:0
                    int w;                                                                            ////__DACE:1:0:0    ////__DACE:1:0:0
                    ////__DACE:1:0:0                                                  ////__DACE:1:0:0
                    ///////////////////                                                               ////__DACE:1:0:0    ////__DACE:1:0:0
                    w=4096;                                                                           ////__DACE:1:0:0    ////__DACE:1:0:0
                    ///////////////////                                                               ////__DACE:1:0:0    ////__DACE:1:0:0
                    ////__DACE:1:0:0                                                  ////__DACE:1:0:0
                    _w = w;                                                                           ////__DACE:1:0:0    ////__DACE:1:0:0
                }                                                                     ////__DACE:1:0:0
                {                                                                     ////__DACE:1:0:2
                    int h;                                                                            ////__DACE:1:0:2    ////__DACE:1:0:2
                    ////__DACE:1:0:2                                                  ////__DACE:1:0:2
                    ///////////////////                                                               ////__DACE:1:0:2    ////__DACE:1:0:2
                    h=2160;                                                                           ////__DACE:1:0:2    ////__DACE:1:0:2
                    ///////////////////                                                               ////__DACE:1:0:2    ////__DACE:1:0:2
                    ////__DACE:1:0:2                                                  ////__DACE:1:0:2
                    _h = h;                                                                           ////__DACE:1:0:2    ////__DACE:1:0:2
                }                                                                     ////__DACE:1:0:2
                init_array_1_0_4(_w, _h, _alpha, &_imgIn[0], _argcount);              ////__DACE:1:0:4
                kernel_deriche_1_0_11(_w, _h, _alpha, &_imgIn[0], &_imgOut[0], &_y1[0], &_y2[0], _argcount);    ////__DACE:1:0:11
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:1:0:22
                    int argc;                                                                         ////__DACE:1:0:22    ////__DACE:1:0:22
                    ////__DACE:1:0:22                                                 ////__DACE:1:0:22
                    ///////////////////                                                               ////__DACE:1:0:22    ////__DACE:1:0:22
                    argc=0;                                                                           ////__DACE:1:0:22    ////__DACE:1:0:22
                    ///////////////////                                                               ////__DACE:1:0:22    ////__DACE:1:0:22
                    ////__DACE:1:0:22                                                 ////__DACE:1:0:22
                    __argc = argc;                                                                    ////__DACE:1:0:22    ////__DACE:1:0:22
                }                                                                     ////__DACE:1:0:22
            } // End omp section
            #pragma omp section
            {
                {                                                                     ////__DACE:1:0:24
                    double argv;                                                                      ////__DACE:1:0:24    ////__DACE:1:0:24
                    ////__DACE:1:0:24                                                 ////__DACE:1:0:24
                    ///////////////////                                                               ////__DACE:1:0:24    ////__DACE:1:0:24
                    argv=0;                                                                           ////__DACE:1:0:24    ////__DACE:1:0:24
                    ///////////////////                                                               ////__DACE:1:0:24    ////__DACE:1:0:24
                    ////__DACE:1:0:24                                                 ////__DACE:1:0:24
                    __argv[0] = argv;                                                                 ////__DACE:1:0:24    ////__DACE:1:0:24
                }                                                                     ////__DACE:1:0:24
            } // End omp section
        } // End omp sections
        delete[] _imgIn;                                                              ////__DACE:1:0:9
        delete[] _imgOut;                                                             ////__DACE:1:0:16
        delete[] _y1;                                                                 ////__DACE:1:0:18
        delete[] _y2;                                                                 ////__DACE:1:0:20
    }                                                                                 ////__DACE:1:0
    
}                                                                                 ////__DACE:0:0:0
////__DACE:0:0:0
void __program_Top_internal(double * __restrict__ _argv, int _argc, int _argcount)
{
    ////__DACE:0
    {                                                                                 ////__DACE:0:0
        
        
        main_0_0_0(_argc, &_argv[0], _argcount);                                      ////__DACE:0:0:0
    }                                                                                 ////__DACE:0:0
}                                                                                 ////__DACE:0
////__DACE:0
DACE_EXPORTED void __program_Top(double * __restrict__ _argv, int _argc, int _argcount)    ////__DACE:0
{                                                                                 ////__DACE:0
    __program_Top_internal(_argv, _argc, _argcount);                                  ////__DACE:0
}                                                                                 ////__DACE:0
////__DACE:0
DACE_EXPORTED int __dace_init_Top(double * __restrict__ _argv, int _argc, int _argcount)    ////__DACE:0
{                                                                                 ////__DACE:0
    int __result = 0;                                                                 ////__DACE:0
    ////__DACE:0
    return __result;                                                                  ////__DACE:0
}                                                                                 ////__DACE:0
////__DACE:0
DACE_EXPORTED void __dace_exit_Top(double * __restrict__ _argv, int _argc, int _argcount)    ////__DACE:0
{                                                                                 ////__DACE:0
}                                                                                 ////__DACE:0

