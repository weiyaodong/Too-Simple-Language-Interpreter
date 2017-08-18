# Too Simple Language Interpreter

### 我需要看这篇文章吗？

这篇文章能告诉你在不考虑性能和其它很多因素的情况下如何写一个能用的玩具解释器。

如果你对这方面不是一无所知，那么可能帮助不是很大。

具体涉及到的知识有 词法分析， 语法分析， 语义分析 和 语法树的求值。

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

所以最后得到的东西大概长这个样子

```bnf
statement ::= expression ";"
	| if_statement
	| while_statement
	| return_statement
	| print_statement
	| variable_definition_statement
	| function_definition_statement
	| block

print_statement ::= "print" expression ";"

block ::= "{" {statement} "}"

if_statement ::= "if" "(" expression ")" statement

while_statement ::= "while" "(" expression ")" statement

return_statement ::= "ret" expression ";"

variable_definition_statement ::= "var" identifier {"=" expression} ";"

function_definition_statement ::= "fun" identifier function_def_parameters_list block

expression ::= term {add_op term}
	| assign_expression

add_op ::= "+" | "-"

term ::= factor {mult_op factor}

mult_op ::= "*" | "/"

factor ::= "(" expression ")"
	| identifier
	| number
	| function_call_expression
	| lambda_expression

function_call_expression ::= identifier function_call_parameters_list
	| "(" expression ")" function_call_parameters_list

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



`Parser` 负责将 // todo