<p style="text-align: center; font-size: 32px; font-weight:bold;">
  IRP
</p>





# IRP

## Known

### Set

| Set | Description                   | Size        | Element         | Remark                                                         |
| ---- | ---------------------- | ----------- | ------------ | :----------------------------------------------------------- |
| $N$ | retailers & depository | $|R|+1$ | $i, j$ | 0 represents depository |
| $R$ | retailers |  | $i,j$ |  |
| $P$ | periods | $3\ or \ 6$ | $t$ | 0 represents initial state |

### Constant

| Constant | Description                          | Type | Range     | Remark             |
| -------- | ------------------------------------ | ---- | --------- | ------------------ |
| $a_i$    | 单位库存持有成本(仓库和客户)         | real |           | $i\in N$           |
| $Q$      | 车的容量                             | real |           |                    |
| $M$      | 无穷大数                             | real | $+\infty$ |                    |
| $B_i$    | 客户 $i$ 的最低库存水平              | real |           | $i\in R$           |
| $C_i$    | 客户 $i$ 的库存容量                  | real |           | $i\in R$           |
| $d_i^t$  | 在周期 $t$ 结点 $i$ 的消耗或补充库存 | real |           | $i\in N,t\in P$    |
| $l_{ij}$ | 边 $(i,j)$ 上的运输费                | real |           | $i,j\in N,i\neq j$ |


## Decision

| Variable     | Description                                                | Type | Domain     | Remark                                                     |
| -------------- | ------------------------------------------------------------ | ---- | ------------- | ------------------------------------------------------------ |
| $I_i^t$ | 结点 $i$ 在周期 $t$ 结束的库存量，t=0表示初始库存 | real |  | $i\in N,t\in P$ |
| $Z_i^t$ | 客户 $i$ 在周期 $t$ 是否被访问 | bool | $\{0,1\}$ | $i\in R,t\in P$ |
| $x_{ij}^t$ | 边 $(i,j)$ 是否在周期 $t$ 被访问   | bool | $\{0,1\}$ | $i,j \in N,i\neq j,t\in P$ |
| $w_i^t$ | 车在周期 $t$ 给客户 $i$ 的送货量 | real |  | $i\in R,t\in P$ |

### Convention and Function

null


## Objective

$$
\begin{split}
Minimize \  \sum_{t\in P}\sum_{i\in N}a_iI_i^t\ +\ \sum_{t\in P}\sum_{i\in N}\sum_{j\in N}l_{ij}x_{ij}^t\ \ , \ \ \ i\neq j
\end{split}\tag{1}
$$

## Constraint

- 只有车从仓库出发，客户才有可能被经过
  $$
  \begin{split}
  Z_i^t-Z_0^t \leqslant0\ \ \ ,\ \ i\in R,t\in P
  \end{split}\tag{2}
  $$



- 车辆不超载
  $$
  \begin{split}
  \sum_{i\in R}w_i^t-Q\leqslant 0 \ \ \ , \ \ \ t\in P
  \end{split}\tag{3}
  $$

- 客户每周期库存不超过客户容量 $C_i$ ，不低于最低库存量 $B_i$ 

  - 如果在周期 $t$ 给客户 $i$ 送货，要满足上界条件：
    $$
    I_i^{t-1}+w_i^t \leqslant C_i\ \ \ ,\ \ if\ Z_i^t=1
    $$

  - 如果在周期 $t$ 不给客户 $i$ 送货，要满足下界条件：
    $$
    I_i^{t-1}-d_i^t \geqslant B_i\ \ \ ,\ \ if \ Z_i^t=0
    $$



  - **综上**

$$
\begin{eqnarray*}
I_i^{t-1}+w_i^t-M(1-Z_i^t)-C_i\leqslant 0 \tag{4} \\
\\ 
I_i^{t-1}-d_i^t+MZ_i^t-B_i\geqslant 0 \tag{5} \\
\\
i\in R,t\in P
\end{eqnarray*}
$$



- 只有车从仓库出发送货，仓库才有库存小于零的可能
  $$
  \begin{split}
  I_0^t-\sum_{i \in R}w_i^t+(1-Z_0^t)M \geqslant 0 \ \ \ , \ \ t\in P
  \end{split}\tag{6}
  $$

- 仓库库存更新
  $$
  \begin{split}
  I_0^t=I_0^{t-1}+d_0^t-\sum_{i\in R}w_i^t\ \ \ , \ \ t\in P
  \end{split}\tag{7}
  $$

- 客户库存更新
  $$
  \begin{split}
  I_i^t=I_i^{t-1}-d_i^t+w_i^t\ \ \ ,\ \ i\in R,t\in P
  \end{split}\tag{8}
  $$

- 车辆形成回路，且保持度约束
  $$
  \begin{eqnarray*}
  \sum_{j\in N}x_{ij}^t=
  \sum_{j\in N}x_{ji}^t=
  Z_i^t \tag{9} \ \ \ \ \ \ 
  i\in N\ ,\ i\neq j\ ,\ t\in P
  \end{eqnarray*}
  $$

- 子回路消除约束
  $$
  \begin{split}
  u_i^t-u_j^t+|N|x_{ij}^t\leqslant |N|-1\ \ \ ,\ \ \ i,j\in R,i\neq j \\
  \\
  t\in P\ \ ,\ 1\leqslant u_i^t\ ,\ u_j^t\leqslant|N|
  \end{split}\tag{10}
  $$





