import pandas as pd
import plotly
import plotly.offline as py
import plotly.graph_objs as go
import csv
import json

py.init_notebook_mode(connected=True)

file_name="D:\\实验室问题\\IRP\\InventoryRouting\\Deploy\\Instance\\Instances_lowcost_H6\\abs1n10.json"
with open(file_name, 'r') as f:
    data = json.load(f)

result_path='D:\\实验室问题\\IRP\\Solution\\Classic\\lowcost_H6.abs1n10.csv'
deliver = []  ##deliver means the result approach
csv_file = csv.reader(open(result_path, 'r'))
for a in csv_file:
    deliver.append(a)

x = []
y = []
kucun = []
produce = []
period = data['periodNum']
for node in data['nodes']:
    x.append(node['x'])
    y.append(node['y'])
    produce.append(-node['unitDemand'])
    kucun.append(node['initQuantity'])


putdown = [0] * len(x)
for i in range(period):
    x1 = []
    y1 = []

    layout=go.Layout(title="period"+str(i+1), xaxis={'title':'x'}, yaxis={'title':'y'})
    for j in range(len(x)):
        kucun[j] = kucun[j] + produce[j]
        putdown[j] = 2
    for k in range(1, len(deliver[3 * i + 1]) - 1):
        num = int(deliver[3 * i + 1][k])
        if (num == 0):
            kucun[num] = kucun[num] - float(deliver[3 * i + 2][k])
        else:
            kucun[num] = kucun[num] + float(deliver[3 * i + 2][k])
            putdown[num] = putdown[num] + float(deliver[3 * i + 2][k])
        x1.append(x[num])
        y1.append(y[num])

    x1.append(x[0])
    y1.append(y[0])
    kucun[0]=100
    putdown[0] = 'black'

    node = go.Scatter(
        x=x,
        y=y,
        mode='markers'
    )
    storage = go.Scatter(
        x=x,
        y=y,
        marker=dict(
            size=kucun,
            color=putdown,
            showscale=True,
            cauto=True,
            opacity=0.8  # transparent
        ),
        mode='markers'
    )

    if (len(x1) != 0):
        route = go.Scatter(
            x=x1,
            y=y1,
        )
        data=[node, storage.copy(), route.copy()]
    else:
        data=[node, storage.copy()]
    figure=go.Figure(data=data,layout=layout)
    py.plot(figure)