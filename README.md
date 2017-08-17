# Too Simple Language Interpreter

### 我需要看这篇文章吗？

这篇文章能告诉你在不考虑性能和其它很多因素的情况下如何写一个能用的玩具解释器。

如果你对这方面不是一无所知，那么可能帮助不是很大。

具体涉及到的知识有 词法分析， 语法分析， 语义分析 和 语法树的求值。

所有的实现都很简单粗暴，目的仅仅是为了让它 work 。



### 这是什么

这是用 `c++` 实现的一个玩具语言 `TSL` 的玩具解释器。

### 为什么是 `C++` 

可能造轮子感比较强，也可能是更熟悉一些。

### `TSL` 是什么

`Too Simple Language` 的缩写。 这是随手起的一个名字，跟 `TypeScript` 没有半毛钱关系。

`TSL` 是一个(不纯的)函数式的而且暂时类型系统什么的啥都没有的玩具语言。

它的 BNF 范式是从数学表达式上扩展而来的，没有参考别的语言。

日后会重构以下，因为现在的实现实在是太蠢了。

写出来的代码大概长这个样子(因为还没有实现注释所以那些看上去像注释的东西实际上是错误的代码)：

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

是的， 很像 `js` ， 我承认了。而且这里的 `block` 本身是没有单独的作用域的，只有函数体会包含一个词法作用域。



### 这个玩具解释器是怎么实现的

我们将整个过程分为 3 个部分，`Tokenize` , `Parse` , `Evaluate` .

`Tokenizer` 把输入的代码转化为 `Token` 流， `Parser`通过 `Token` 流生成抽象语法树 `AST` , 再对 `AST` 进行求值就完成了我们需要做的 “解释” 这个工作。

// todo 