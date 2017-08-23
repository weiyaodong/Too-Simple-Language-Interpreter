# Too Simple Language Interpreter

跑起来大概长这样：

![TSLI-1](http://zjuwyd.com/files/images/TSLI/TSLI1.png)

项目地址在 [这里](https://github.com/weiyaodong/Too-Simple-Language-Interpreter) 。

UPDATE 1:

更改了循环流程控制，添加了 `for` 循环、  `break` 和 `continue` 。

UPDATE 2:

更改了 `bnf` 文法，使其支持一定的后缀表达式，例如下标访问、函数连续调用、成员访问等功能。

添加了指向函数自己的 `this` 指针。

添加了成员访问符 `.` ，可以通过其访问函数闭包内部的变量。

更改了 `asn_expr` 的文法，使之支持直接对闭包成员的修改和定义。

<!-- more -->

### 我需要看这篇文章吗？

这篇文章能告诉你在不考虑性能和其它很多因素的情况下如何写一个能用的玩具解释器。

如果你对这方面不是一无所知，那么可能帮助不是很大。

具体涉及到的知识有 简单的语法分析， 语义分析 和 语法树的求值。

所有的实现都很简单粗暴，目的仅仅是为了让它 work 。



### 这是什么

这是用 `c++` 实现的一个玩具语言 `TSL` 的玩具解释器。

### `TSL` 是什么

`Too Simple Language` 的缩写。 这是随手起的一个名字，跟 `TypeScript` 没有半毛钱关系。

`TSL` 是一个(不纯的)函数式的而且暂时类型系统什么的啥都没有的玩具语言。

它的 BNF 范式是从数学表达式上扩展而来的，没有参考别的语言。

日后会重构一下，现在的实现实在是太蠢了。

写出来的代码大概长这个样子(还没有实现注释所以那些看上去像注释的东西实际上是错误的代码)：

```javascript
{
	// define a variable named "x" and initialize it with integer 0
	var x = 0;

	// arithmetic expression can be evaluated correctly 
	x = (1 + 2) * (3 + 4) / 7; // x = 3

	// assign x as a lambda expression "() => 1"
	x = () => 1;

	// lambda expression can be like this, too
	x = () => {
		ret 1;
	};

	// evaluate the lambda expression and print it
	print x(); // output : 1

	// or print the lambda expression itself
	print x;   // output : () => 1

	var sum = 50;
	var n = 10;

	// while statement 
	while (n) {
		sum = sum + n;
		n = n - 1;
	}
	print sum; // output : 55

	// and if statement
	if (1) {
		sum = sum + 1;
	}
	print sum; // output : 56  	

	// one way to define function
	fun add(x, y) => {
		ret x + y;
	}

	// function can be curried automatically
	var add1 = add(1);
	print add1(1); // output : 2

	// or high-order function
	var pack = (f) => () => f;
	var unpack = (f) => f();  	
	print unpack(pack(1)); // output : 1  

  	// update : 新增了弱类型的大小比较啥的还有 for 循环以及 break 和 continue
  	
  	for (var i = 1; i <= 10; i += 1) if (i == 5) break;
  	
  	print x; // 5
  
	var a = () => a; // we get a function a
	print a()()()()()()()(); // output : () => a
	
	// or we can write
	a = () => this;
	print a()()()()()()()(); // output : () => this
	
	a.x = 1; // we defined a new variable in a's closure
	print a.x; // output : 1
	
	a = () => x; // this is the global x
	print a(); // output : 5
	
	a.x = 1; // we defined a new "x" in this closure
	print a(); // output : 1
}
```

是的， 很像 `js` ， 我承认了。而且这里的 `block` 本身是没有单独的作用域的，只有函数会包含一个词法作用域。

它的文法我是从一个算术表达式的 `bnf` 扩展而来。原来是这个样子：

```bnf
expr ::= term {add_op term}
add_op ::= "+" | "-"

term ::= factor {mult_op factor}
mult_op ::= "*" | "/"

factor ::= number | "(" expr ")"
```

事实上多一个符号优先级，就需要多一些 `bnf` 元素。为了简便，我们当前只实现了两个优先级。
update : 新增的运算符将优先级扩展到 7 个，但在处理时只需要对 `Parser` 做小规模修改即可

所以最后得到的东西大概长这个样子

```bnf
statement ::= expression ";"
	| if_statement
	| while_statement
	| for_statement
	| return_statement
	| break_statement
	| continue_statement
	| print_statement
	| variable_definition_statement
	| function_definition_statement
	| block

print_statement ::= "print" expression {"," expression} ";"

block ::= "{" {statement} "}"

if_statement ::= "if" "(" expression ")" statement

while_statement ::= "while" "(" expression ")" statement

for_statement ::= "for" "(" var_def_expr ";" expression ";" expression ")" statement
var_def_expr ::= "var" identifier ("=" expression) {"," identifier ("=" expression)}

return_statement ::= "ret" expression ";"

variable_definition_statement ::= "var" identifier {"=" expression} ";"

function_definition_s tatement ::= "fun" identifier function_def_parameters_list block

primary_expression ::= "(" expression ")"
	| identifier
	| number
	| boolean
	| lambda_expression
	| list_expression

postfix_expression ::= primary_expression
	| function_call_expression
	| list_visit_expression
	| member_expression

member_expression ::= postfix_expression "." identifier

function_call_expression ::= postfix_expression function_call_parameters_list
	| "(" expression ")" function_call_parameters_list

list_expression ::= "[" (expression {"," expression}) "]"

list_visit_expression ::=  postfix_expression "[" expression "]"

mult_expr ::= postfix_expression {mult_op factor}
mult_op ::= "*" | "/"

add_expr ::= mult_expr {add_op mult_expr}
add_op ::= "+" | "-"

relat_expr ::= add_expr (relat_op add_expr)
relat_op ::= ">" | "<" | ">=" | "<="

eq_expr ::= relat_expr (eq_op relat_expr)
eq_op ::= "==" | "!="

and_expr ::= eq_expr {and_op eq_expr}
and_op ::= "&&"

or_expr ::= and_expr {or_op and_expr}
or_op ::= "||"

asn_expr ::= postfix_expression asn_op expression

expression ::= asn_expr
	| or_expr

lambda_expression ::= function_def_parameters_list "=>" statement

function_def_parameters_list ::= "(" [identifier] ")"
	| "(" identifier {"," identifier} ")"

function_call_parameters_list ::= "(" [expression] ")"
	| "(" expression {"," expression} ")"
```

这个文法是有一定的问题的，它不能实现像 `fun()()()` 这样的连续调用而必须 `(((fun())()))()` 这样加上括号。(// todo)

### 这个玩具解释器是怎么实现的

我们将整个过程分为 3 个部分，`Tokenize` , `Parse` , `Evaluate` .

`Tokenizer` 把输入的代码转化为 `Token` 流， `Parser`通过 `Token` 流生成抽象语法树 `AST` , 再对 `AST` 进行求值就完成了我们需要做的 “解释” 这个工作。



这里实现的 `Tokenizer` 存在感不强，很多信息都没有被 `parser` 获取就没了。(最开始只是想写一个 `Tokenizer`)

我们没有使用 `NFA` 和 `DFA` 那一套理论来写，使用的全部是朴素的算法。

`Tokenizer` 具体的工作是这样的 ：

`"var x = 1;"`  => `Tokenizer`  =>  `<keyword: var> <ident: x> <oper: => <num: 1> <;>`

这里的实现没有任何需要用到 c 小程以外的知识的地方。



`Parser` 负责将 `Token` 转化为 `Abstract Syntax Tree` 也就是语法树。`AST` 揭示了代码的语言无关的结构，通过 `AST` 我们可以轻易地把代码从一种语言转译到另外一种语言。

实际上在完成了 `Tokenizer` 之后， `Parser` 所需要做的事情仅仅是对着 `bnf` 文法一条条的写而已。



而可能最为麻烦的部分可能是语法树的求值，也就是程序的运行。

目前我仅仅提供了 `Integer` 和 `Function` 这两种类型。为了简便我把它们都放在了 `Object` 这一个类里来表示 `TSL` 中的一个**值**。

`Integer` 本身只需要存一个整数就好，没什么麻烦的。而有一定思考难度的东西就在于 `Function` 的储存。

`Function` 作为一个函数，它有它的参数 `Parameters` ，也有相应的函数体 `Body`，以及对应的词法作用域 `Scope` 。

`parameters` 的类型是 `std::string`，即在函数调用时参数们需要绑定的名字  ，`body` 指向一个语法树的节点用来表示函数体 , 而 `scope` 则指向这个函数独一无二的词法作用域。

先谈一下 `Scope` 的实现。一个 `Scope` 包含一张变量表，用来保存在当前定义域下定义过的变量，并有一个父节点指针，用来指向父作用域，以及一个子节点域，用来存它对应的子节点。`Scope` 需要做的仅仅是寻找一个变量，定义一个变量和修改一个变量。

函数的调用需要结合当前的语境，也就是需要一个 `Scope` 来作为求值的参数。

在调用时，如果是 `Partial Apply` 即应用的参数数量小于函数的期望参数数量时，我们需要返回一个新的函数，这个函数的 `body` 与当前函数一样，而需要修改的是 `parameters` 和 `scope` 。因为已经有部分参数已经绑定了相应的值，所以新的函数中我们会去掉这些参数并把它们定义在新的 `scope` 中。而如果调用时应用了所有的参数，那么我们只需要在新的 `scope` 里对 `body` 里面的语法树进行求值就可以了。

关于资源管理方面的问题，因为没有实现 `Garbage Collect` , 所以我们让所有的定义的变量作为指针存在 `scope` 里，而临时函数的 `scope` 会挂在其父作用域下。



对于返回语句和循环结构的控制，这里简单粗暴地使用了异常处理来抛出函数的结果以及跳出循环。

具体的实现可能会有一些细节问题，可以结合代码进行参考。



TODO:

读入操作，字符和字符串，数组，固定类型的变量，常量定义，高精度整数……

