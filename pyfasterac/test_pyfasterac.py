#! python3

import pyfasterac as pyf

make_plot = False

if make_plot:
    import matplotlib.pyplot as plt

print("fasterac version: ", pyf.fast_version())

if make_plot:
    Q_coinc_dict = {
        6: [],
        8: []
    }

    Q_alone_dict = {
        5: [],
        6: [],
        7: [],
        8: []
    }

counter_events = 0
r = pyf.fastreader("./pouette_0001.fast")
while r.get_next_event():
    # if r.is_group():
    event = r.get_event()
    print(f"Event {counter_events}:", event.label, event.time, event.multiplicity)
    if event.multiplicity == 2:
        for sub_event in event.sub_events:
            if make_plot:
                Q_coinc_dict[sub_event.label].append(sub_event.q)
            print("\t", sub_event.label, "dt=",sub_event.delta_t, "q=",sub_event.q)
    elif event.multiplicity < 2:
        for sub_event in event.sub_events:
            if sub_event.label > 999: continue
            if make_plot:
                Q_alone_dict[sub_event.label].append(sub_event.q)
            print("\t", "dt=",sub_event.delta_t, "q=",sub_event.q)
    counter_events+=1

print(f"Number of events: {counter_events}")

if make_plot:
    plt.scatter(Q_coinc_dict[6], Q_coinc_dict[8], marker = ".")
    plt.savefig("coinc.pdf")
    for key, list in Q_alone_dict.items():
        plt.clf()
        plt.hist(list, bins=1000, range=[0,1e6])
        plt.savefig(f"plot_label{key}.pdf")
