#ifndef _UNIVERSAL_SHADER_ERROR_H_
#define _UNIVERSAL_SHADER_ERROR_H_

#define ERROR_MSG__MISSING_SEMICOLON(_token)      m_pMsg->WriteErrorW(TRUE, (_token).offset(), 1000)
#define ERROR_MSG__MISSING_OPENBRACKET    CLBREAK
#define ERROR_MSG__MISSING_CLOSEDBRACKET  CLBREAK

//
// 竟然可以使用中文 ಠ౪ಠ
//
#define ERROR_MSG_缺少分号(_token)    ERROR_MSG__MISSING_SEMICOLON(_token)
#define ERROR_MSG_缺少开括号  ERROR_MSG__MISSING_OPENBRACKET  
#define ERROR_MSG_缺少闭括号  ERROR_MSG__MISSING_CLOSEDBRACKET
//#define ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动 CLBREAK
//#define ERROR_MSG_C1021_无效的预处理器命令 

#define E1004_意外的文件结束            1004
#define E1016_ifdef_应输入标识符        1016
#define E1017_无效的整数常数表达式      1017
#define E1018_意外的_elif              1018
#define E1019_意外的_else              1019
#define E1020_意外的_endif             1020
#define E1021_无效的预处理器命令_vs     1021

#define E1071_在注释中遇到意外的文件结束 1071

#define E1189_用户定义错误_vs           1189

#define E2004_应输入_defined_id_                                     2004
#define E2007_define_缺少定义                                        2007
#define E2008_宏定义中的意外                                          2008
#define E2010_宏形参表中的意外                                        2010
#define E2012_在左尖括号之后缺少名称                                  2012
#define E2013_缺少右尖括号                                           2013
#define E2014_预处理器命令必须作为第一个非空白空间启动                 2014
#define E2017_非法的转义序列                                         2017
#define E2018_未知字符hexnumber                                     2018
#define E2019_应找到预处理器指令_却找到character                      2019
#define E2021_应输入指数值而非character                              2021
#define E2022_number_对字符来说太大                                  2022
#define E2030_identifier_结构_联合成员重定义                          2030
#define E2039_identifier1_不是identifier2的成员                      2039
#define E2041_非法的数字character_用于基number                        2041
#define E2043_非法_break                                                  2043
#define E2044_非法_continue                                               2044
#define E2046_非法的_case                                                 2046
#define E2047_非法的_default                                              2047
#define E2054_在identifier之后应输入左括号                                 2054
#define E2055_应输入形参表_而不是类型表                                    2055
#define E2056_非法表达式                                                  2056
#define E2057_应输入常数表达式                                             2057
#define E2059_语法错误_vs                                              2059
#define E2060_语法错误_遇到文件结束_至少还需要一个标记                  2060
#define E2061_语法错误_标识符identifier                                 2061
#define E2062_意外的类型_vs                                            2062
#define E2063_identifier_不是函数                                      2063
#define E2064_项不会计算为接受number个参数的函数                          2064
#define E2071_identifier_非法的存储类                                  2071
#define E2075_identifier_数组初始化需要大括号                           2075
#define E2078_初始值设定项太多                                             2078
#define E2079_identifier使用未定义的类_结构_联合name                   2079
#define E2081_identifier_形参表中的名称非法                             2081
#define E2082_形参identifier的重定义                                    2082
#define E2084_函数function已有主体                                      2084
#define E2085_identifier_不在形参表中                                  2085
#define E2086_identifier_重定义                                        2086
#define E2087_identifier_缺少下标                                      2087
#define E2088_operator_对于class_key非法                             2088
#define E2090_函数返回数组                                                2090
#define E2099_初始值设定项不是常数                                         2099
#define E2106_operator_左操作数必须为左值                               2106
#define E2108_下标不是整型                                                2108
#define E2109_下标要求数组或指针类型                                       2109
#define E2111_指针加法要求整型操作数                                  2111
#define E2112_指针减法要求整型或指针类型操作数                         2112
#define E2118_负下标                                                      2118
#define E2120_对于所有类型void非法                                       2120
#define E2121_无效字符_可能是宏展开的结果                            2121
#define E2122_identifier_名称列表中的原型参数非法                       2122
#define E2124_被零除或对零求模                                             2124
#define E2126_operator_不正确的操作数                                   2126
#define E2130_line_应输入包含文件名的字符串_却找到token                 2130
#define E2132_语法错误_意外的标识符                                      2132
#define E2133_identifier_未知的大小                                    2133
#define E2141_数组大小溢出                                                2141
#define E2144_语法错误_type的前面应有token                            2144
#define E2145_语法错误_标识符前面缺少token                              2145
#define E2146_语法错误_标识符identifier前缺少token                   2146
#define E2147_语法错误_identifier是新的关键字                            2147
#define E2153_十六进制常数必须至少有一个十六进制数字                         2153
#define E2156_杂注必须在函数的外部                                         2156
#define E2159_指定了一个以上的存储类                                       2159
#define E2160_双井号不能在宏定义的开始处出现                                 2160
#define E2161_宏定义以标记粘贴运算符双井号结束                            2161
#define E2162_应输入宏形参                                                2162
#define E2166_左值指定常数对象                                             2166
#define E2171_operator_type类型的操作数非法                            2171
#define E2177_常数太大                                                    2177
#define E2180_控制表达式的类型为type                                     2180
#define E2181_没有匹配_if_的非法_else                                      2181
#define E2186_operator_void类型的操作数非法                           2186
#define E2189_error_vs                                                2189
#define E2197_function_用于调用的参数太多                               2197
#define E2198_function_用于调用的参数太少                               2198
#define E2199_语法错误_在全局范围内找到identifier                       2199
#define E2206_function_typedef_不能用于函数定义                         2206
#define E2208_type_没有使用此类型进行定义的成员                          2208
#define E2220_视为错误的警告_没有生成对象文件                              2220
#define E2226_语法错误_意外的type类型                                   2226
#define E2228_identifier的左侧必须有类_结构_联合                        2228
#define E2229_类型identifier有非法的零大小的数组                         2229
#define E2232_左操作数具有class_key类型_使用点号                   2232
#define E2233_identifier_包含零大小的数组的对象的数组是非法的            2233
#define E2234_name_引用数组是非法的                                     2234
#define E2236_意外的class_keyidentifier_您是否忘记了分号             2236
#define E2238_token前有意外的标记                                        2238
#define E2244_identifier_无法将函数定义与现有的声明匹配                  2244
#define E2267_function_具有块范围的静态函数非法                         2267
#define E2274_type_位于点号运算符右边非法                                2274
#define E2275_identifier_将此类型用作表达式非法                         2275
#define E2289_多次使用同一类型限定符                                       2289
#define E2295_转义的character_在宏定义中非法                            2295
#define E2296_operator_左操作数错误                                    2296
#define E2297_operator_右操作数错误                                     2297


//////////////////////////////////////////////////////////////////////////
// 自定义错误码

#define E1050_缺少类型描述              1050

#define E4006_undef应输入标识符         4006
#define E4067_预处理器指令后有意外标记_应输入换行符 4067
#define E2004_应输入_defined_id        2004
#define E2007_define缺少定义_vs         2007
#define E2008_宏定义中的意外_vs         2008
#define E2010_宏形参表中的意外          2010
#define E2059_SyntaxError_vs           2059
#define E2121_无效的井号_可能是宏扩展    2121
#define E9999_未定义错误_vsd            9999




#endif // _UNIVERSAL_SHADER_ERROR_H_