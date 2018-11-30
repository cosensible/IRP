import plotly.offline as pol
import plotly.graph_objs as go

import csv
import json

pol.init_notebook_mode(connected=True)

# 载入算例
instance_path='./Instance/Instances_lowcost_H6/abs1n25.json'
with open(instance_path, 'r') as f:
    instance = json.load(f)

# 载入解决方案
deliveries=[]
solution_path='./Classic/lowcost_H6.abs1n25.csv'
csv_file = csv.reader(open(solution_path, 'r'))
for line in csv_file:
    deliveries.append(line)

retailers=[]
inventories=[]
for n in instance['nodes']:
    if n['id']==0:
        continue
    retailers.append('n'+str(n['id']))
    inventories.append(n['initQuantity'])

trace0 = go.Bar(
    x=inventories.copy(),
    y=retailers,
    orientation = 'h',
    text=inventories.copy(),
    textposition = 'auto',
    marker=dict(
    line=dict(
        color='rgb(8,48,107)',
        width=1.5),
    ),
    opacity=0.6,
    name='t0',
)
bars=[trace0]

for t in range(0,instance['periodNum']):
    for n in instance['nodes']:
        if n['id']==0:
            continue
        inventories[n['id']-1]=inventories[n['id']-1]-n['unitDemand']
    i=2
    for k in deliveries[3*t+1][2:-1]:
        inventories[int(k)-1]=inventories[int(k)-1]+float(deliveries[3*t+2][i])
        i+=1
    bars.append(go.Bar(
        x=inventories.copy(),
        y=retailers,
        orientation = 'h',
        text=inventories.copy(),
        textposition = 'auto',
        marker=dict(
            line=dict(
            # color='rgb(8,48,107)',
            width=1.5),
        ),
        opacity=0.6,
        name='t'+str(t+1)))

layout = go.Layout(
    barmode='group',
    bargroupgap=0.1,
    bargap=0.15,
)

fig = go.Figure(data=bars, layout=layout)
pol.plot(fig, filename='inventory_change')