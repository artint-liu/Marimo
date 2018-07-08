#ifndef _UNIVERSAL_SHADER_ERROR_H_
#define _UNIVERSAL_SHADER_ERROR_H_

#define ERROR_MSG__MISSING_SEMICOLON(_token)      m_pMsg->WriteErrorW(TRUE, (_token).offset(), UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”"))
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
#define E2059_SyntaxError_vs                                           2059
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
#define E2145_语法错误_标识符前面缺少token_vs                              2145
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

namespace UVShader
{
  enum ErrorCode
  {
    E1004 = 1004, // "遇到意外的文件结束";
    E1016 = 1016, // "#ifdef 应输入标识符";
    E1017 = 1017, // "无效的整数常数表达式";
    E1018 = 1018, // "意外的 #elif";
    E1019 = 1019, // "意外的 #else";
    E1020 = 1020, // "意外的 #endif";
    E1021 = 1021, // "无效的预处理器命令 “%s”.";

    E1070 = 1070, // "";
    E1071 = 1071, // "在注释中遇到意外的文件结束";
    E1083 = 1083, // "";
    E1189 = 1189, // "#error : %s";

    E2004 = 2004, // "应输入“defined(id)”";
    E2007 = 2007, // "#define 缺少定义.";
    E2008 = 2008, // "“%s”:宏定义中的意外";
    E2010 = 2010, // "“%s”:宏形参表中的意外";
    E2012 = 2012, // "在“<”之后缺少名称";
    E2013 = 2013, // "缺少“>”";
    E2014 = 2014, // "预处理器命令必须作为第一个非空白空间启动";
    E2017 = 2017, // "非法的转义序列";
    E2018 = 2018, // "未知字符“hexnumber”";
    E2019 = 2019, // "应找到预处理器指令，却找到“character”";
    E2021 = 2021, // "应输入指数值，而非“character”";
    E2022 = 2022, // "“number”: 对字符来说太大";
    E2030 = 2030, // "“identifier”: 结构/联合成员重定义";
    E2039 = 2039, // "“identifier1”: 不是“identifier2”的成员";
    E2041 = 2041, // "非法的数字“character”(用于基“number”)";
    E2043 = 2043, // "非法 break";
    E2044 = 2044, // "非法 continue";
    E2046 = 2046, // "非法的 case";
    E2047 = 2047, // "非法的 default";
    E2054 = 2054, // "在“identifier”之后应输入“(”";
    E2055 = 2055, // "应输入形参表，而不是类型表";
    E2056 = 2056, // "非法表达式";
    E2057 = 2057, // "应输入常数表达式";
    E2059 = 2059, // "语法错误 :“%s”";
    E2060 = 2060, // "语法错误 : 遇到文件结束, 至少还需要一个标记。";
    E2061 = 2061, // "语法错误: 标识符“identifier”";
    E2062 = 2062, // "意外的类型“%s”";
    E2063 = 2063, // "“identifier”: 不是函数";
    E2064 = 2064, // "项不会计算为接受“number”个参数的函数";
    E2071 = 2071, // "“identifier”: 非法的存储类";
    E2075 = 2075, // "“identifier”: 数组初始化需要大括号";
    E2078 = 2078, // "初始值设定项太多";
    E2079 = 2079, // "“identifier”使用未定义的类/结构/联合“name”";
    E2081 = 2081, // "“identifier”: 形参表中的名称非法";
    E2082 = 2082, // "形参“identifier”的重定义";
    E2084 = 2084, // "函数“function”已有主体";
    E2085 = 2085, // "“identifier”: 不在形参表中";
    E2086 = 2086, // "“identifier”: 重定义";
    E2087 = 2087, // "“identifier”: 缺少下标";
    E2088 = 2088, // "“operator”: 对于“class-key”非法";
    E2090 = 2090, // "函数返回数组";
    E2099 = 2099, // "初始值设定项不是常数";
    E2106 = 2106, // "“operator”: 左操作数必须为左值";
    E2108 = 2108, // "下标不是整型";
    E2109 = 2109, // "下标要求数组或指针类型";
    E2111 = 2111, // "“+”: 指针加法要求整型操作数";
    E2112 = 2112, // "“-”: 指针减法要求整型或指针类型操作数";
    E2118 = 2118, // "负下标";
    E2120 = 2120, // "对于所有类型“void”非法";
    E2121 = 2121, // "“#”: 无效字符 : 可能是宏展开的结果";
    E2122 = 2122, // "“identifier”: 名称列表中的原型参数非法";
    E2124 = 2124, // "被零除或对零求模";
    E2126 = 2126, // "“operator”: 不正确的操作数";
    E2130 = 2130, // "#line 应输入包含文件名的字符串，却找到“token”";
    E2132 = 2132, // "语法错误 : 意外的标识符";
    E2133 = 2133, // "“identifier”: 未知的大小";
    E2141 = 2141, // "数组大小溢出";
    E2144 = 2144, // "语法错误 :“{type}”的前面应有“{token}”";
    E2145 = 2145, // "语法错误：标识符前面缺少“%s”";
    E2146 = 2146, // "语法错误 : 标识符“identifier”前缺少“token”";
    E2147 = 2147, // "语法错误:“identifier”是新的关键字";
    E2153 = 2153, // "十六进制常数必须至少有一个十六进制数字";
    E2156 = 2156, // "杂注必须在函数的外部";
    E2159 = 2159, // "指定了一个以上的存储类";
    E2160 = 2160, // "“##”不能在宏定义的开始处出现";
    E2161 = 2161, // "宏定义以标记粘贴运算符 (##) 结束。";
    E2162 = 2162, // "应输入宏形参";
    E2166 = 2166, // "左值指定常数对象";
    E2171 = 2171, // "“operator”:“type”类型的操作数非法";
    E2177 = 2177, // "常数太大";
    E2180 = 2180, // "控制表达式的类型为“type”";
    E2181 = 2181, // "没有匹配 if 的非法 else";
    E2186 = 2186, // "“operator”: “void”类型的操作数非法";
    E2189 = 2189, // "#error : %s";
    E2197 = 2197, // "“function”: 用于调用的参数太多";
    E2198 = 2198, // "“function”: 用于调用的参数太少";
    E2199 = 2199, // "语法错误 : 在全局范围内找到“identifier (”(是有意这样声明的吗？)";
    E2206 = 2206, // "“function”: typedef 不能用于函数定义";
    E2208 = 2208, // "“type”: 没有使用此类型进行定义的成员";
    E2220 = 2220, // "视为错误的警告，没有生成对象文件";
    E2226 = 2226, // "语法错误: 意外的“type”类型";
    E2228 = 2228, // "“.identifier”的左侧必须有类/结构/联合";
    E2229 = 2229, // "类型“identifier”有非法的零大小的数组";
    E2232 = 2232, // "“–>”: 左操作数具有“class-key”类型，使用“.”";
    E2233 = 2233, // "“identifier”: 包含零大小的数组的对象的数组是非法的";
    E2234 = 2234, // "“name”: 引用数组是非法的";
    E2236 = 2236, // "意外的“class-key”“identifier”。您是否忘记了“;”？";
    E2238 = 2238, // "“token”前有意外的标记";
    E2244 = 2244, // "“identifier”: 无法将函数定义与现有的声明匹配";
    E2267 = 2267, // "“function”: 具有块范围的静态函数非法";
    E2274 = 2274, // "“type”: 位于“.”运算符右边非法";
    E2275 = 2275, // "“identifier”: 将此类型用作表达式非法";
    E2289 = 2289, // "多次使用同一类型限定符";
    E2295 = 2295, // "转义的“character”: 在宏定义中非法";
    E2296 = 2296, // "“operator”: 左操作数错误";
    E2297 = 2297, // "“operator”：右操作数错误";

    E2332 = 2332, // "“typedef”: 缺少标记名";
    E2333 = 2333, // "“function”: 函数声明中有错误；跳过函数体";
    E2334 = 2334, // "': 或 {' 的前面有意外标记；跳过明显的函数体";
    E2337 = 2337, // "“attribute name”: 未找到属性";
    E2369 = 2369, // "“array”: 重定义；不同的下标";
    E2370 = 2370, // "“identifier”: 重定义；不同的存储类";
    E2371 = 2371, // "“identifier”: 重定义；不同的基类型";
    E2374 = 2374, // "“identifier”: 重定义；多次初始化";
    E2375 = 2375, // "“function”: 重定义；不同的链接";
    E2377 = 2377, // "“identifier”: 重定义；typedef 不能由任何其他符号重载";
    E2378 = 2378, // "“identifier”: 重定义；符号不能由 typedef 重载";
    E2414 = 2414, // "非法的操作数数目";
    E2415 = 2415, // "不正确的操作数类型";
    E2417 = 2417, // "在“context”中被 0 除";
    E2419 = 2419, // "在“context”中对 0 求模";
    E2420 = 2420, // "“identifier”: 上下文中的非法符号";
    E2424 = 2424, // "“token”:“context”中的表达式不正确";
    E2426 = 2426, // "“token”:“context”中的非法运算符";
    E2428 = 2428, // "“operation”: 在“bool”类型的操作数上不允许";
    E2433 = 2433, // "“identifier”: 不允许在数据声明中使用“modifier”";
    E2446 = 2446, // "“operator”: 没有从“type1”到“type2”的转换";
    E2447 = 2447, // "“{”: 缺少函数标题(是否是老式的形式表？)";
    E2448 = 2448, // "“identifier”: 函数样式初始值设定项类似函数定义";
    E2449 = 2449, // "在文件范围内找到“{”(是否缺少函数头？)";
    E2450 = 2450, // "“type”类型的 switch 表达式是非法的";
    E2451 = 2451, // "“type”类型的条件表达式是非法的";
    E2457 = 2457, // "“macro”: 预定义的宏不能出现在函数体的外部";
    E2458 = 2458, // "“identifier”: 定义范围内的重定义";
    E2459 = 2459, // "“identifier”: 正被定义；无法作为匿名成员添加";
    E2460 = 2460, // "“identifier1”: 使用正在定义的“identifier2”";
    E2466 = 2466, // "不能分配常数大小为 0 的数组";
    E2470 = 2470, // "“function”: 看起来像函数定义，但没有参数列表；跳过明显的函数体";
    E2499 = 2499, // "“class”: 类不能是其自身的基类";
    E2500 = 2500, // "“identifier1”:“identifier2”已是直接基类";
    E2504 = 2504, // "“class”: 未定义基类";
    E2513 = 2513, // "“type”: 在“=”前没有声明变量";

    //E9999 = 9999, // 未定义错误
  };
} // namespace UVShader

//////////////////////////////////////////////////////////////////////////
// 自定义错误码

#define E1050_缺少类型描述              1050

#define E4006_undef应输入标识符         4006
#define E4067_预处理器指令后有意外标记_应输入换行符 4067
#define E2004_应输入_defined_id        2004
#define E2007_define缺少定义_vs         2007
#define E2008_宏定义中的意外_vs         2008
#define E2010_宏形参表中的意外          2010

//#define E9999_未定义错误_vsd            9999




#endif // _UNIVERSAL_SHADER_ERROR_H_