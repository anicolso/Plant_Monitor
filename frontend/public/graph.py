import pandas as pd
import schedule
import matplotlib.pyplot as plt
from datetime import datetime



def graph_data():
    path = '/home/sami/cmpt433/public/myApps/'
    light_data = pd.read_csv(path + 'light.txt', sep=',', header=None,names=['voltage', 'time'])
    humidity_data = pd.read_csv(path + 'humidity.txt', sep=',', header=None,names=['voltage', 'time'])
    temperature_data = pd.read_csv(path + 'temperature.txt', sep=',', header=None,names=['voltage', 'time'])

    now = datetime.now()
    light_label = len(light_data)//3
    if(light_label == 0):
        light_label = 1
    
    humidity_label = len(humidity_data)//3
    if(humidity_label == 0):
        humidity_label = 1
    
    temperature_label = len(temperature_data)//3
    if(temperature_label == 0):
        temperature_label = 1

    # create light graph
    plt.figure(1)
    plt.plot(light_data['time'], light_data['voltage'])
    plt.xticks(light_data['time'][::light_label])
    plt.xlabel("time")
    plt.ylabel("voltage")
    plt.savefig('lightfig-' + str(now.strftime("%d-%m-%Y-%H:%M")) + '.png')

    # create humidity graph
    plt.figure(2)
    plt.plot(humidity_data['time'], humidity_data['voltage'])
    plt.xticks(humidity_data['time'][::humidity_label])
    plt.xlabel("time")
    plt.ylabel("voltage")
    plt.savefig('humidityfig-' + str(now.strftime("%d-%m-%Y-%H:%M")) + '.png')


    # create temperature graph
    plt.figure(3)
    plt.plot(temperature_data['time'], temperature_data['voltage'])
    plt.xticks(temperature_data['time'][::temperature_label])
    plt.xlabel("time")
    plt.ylabel("voltage")
    plt.savefig('tempfig-' + str(now.strftime("%d-%m-%Y-%H:%M")) + '.png')



schedule.every(1).minutes.at(":00").do(graph_data)

while True:
    schedule.run_pending()
    