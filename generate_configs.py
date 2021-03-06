import json

def generate_configs(file_prefix=False, num_events=100, duration=1e-4, dfdt=1e8, power=1e-15, spacing=8e-5):
    '''
        generate a config json file for locust input

        <num_events> is number of chirps
        <duration> is time length of each chirp [s]
        <dfdt> is the chirp rate [Hz/s]
        <power> is the signal power
        <spacing> is the time between the end of one chirp and the start of 
                  the next. [s]
    '''
    sampling_rate = 200e6
    if not file_prefix:
        file_prefix = (str(num_events) + 'events_' + str(duration) + 'dur_' +
            str(dfdt) + 'dfdt_' + str(power) + 'power')
    outfile = open(file_prefix + '_config.json', 'w')
    out_dict = {
        "transfer_function_filename":
            "/home/laroque/Repos/locust_mc/receiver_transfer_functions.json",
        "amp1_noise_temperature": 5,
        "amp2_noise_temperature": 30,
        "receiver1_noise_temperature": 15,
        "receiver2_noise_temperature": 30,
        "BField": 0.9,
        "waveguide_length": 147.19,
        "waveguide_setup": "SINGLEAMP",
        "distance_to_short": 5,
        "phase_delay_length": 5,
        "hf_mixing_frequency": 24.2e9,
        "lf_mixing_frequency": 500e6,
        "datafile_duration": num_events * (duration + spacing) + spacing,
        "egg_outfile_name": file_prefix + ".egg",
        "mcinfo_outfile_name": file_prefix + ".mcinfo",
        "number_of_events": num_events,
        "events": []
    }
    event_time = spacing
    for start in range(num_events):
        out_dict['events'].append({
            "start_time": event_time,
            "duration": duration,
            "start_frequency": 24.750e9,
            "dfdt": dfdt,
            "power": power
            })
        event_time += duration + spacing
    json.dump(out_dict, outfile, indent=4)

if __name__ == '__main__':
    durrations = [200e-6, 100e-6, 50e-6, 25e-6, 5e-6]
    powers = [1e-15, 7e-16, 4e-16, 1e-16, 7e-17, 4e-17, 1e-17, 7e-18, 4e-18, 1e-18, 7e-19, 4e-19, 1e-19]
    chrates = [1e8, 3e8]
    for chrate in chrates:
        for dur in durrations:
            for powr in powers:
                generate_configs(num_events=500, duration=dur, dfdt=chrate, power=powr)
