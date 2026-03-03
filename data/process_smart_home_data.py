import pandas as pd
import numpy as np
import os
import random
import argparse
import glob


def split_and_clean_dataset(input_file='smartHomeEnergyManagement.csv'):
    """
    Step 1: Split the large dataset by home_id and clean each file
    """
    print("\n=== STEP 1: SPLITTING AND CLEANING DATASET ===")
    
    # 1. Loading the large file
    df = pd.read_csv(input_file, sep=',')
    
    # 2. Get the list of all unique home_id
    unique_homes = df['home_id'].unique()
    
    print(f"Detected homes: {unique_homes}")
    
    # 3. Loop to create one file per home
    temp_files = []
    for h_id in unique_homes:
        # Filter data for this specific home
        home_df = df[df['home_id'] == h_id]
        
        # Generate the file name (e.g.: home_1.csv, home_2.csv)
        file_name = f"home_{h_id}.csv"
        temp_files.append(file_name)
        
        # Save
        home_df.to_csv(file_name, index=False, sep=',')
        print(f"File created: {file_name} ({len(home_df)} rows)")
    
    print("\nSplit completed!")
    
    # 4. Cleaning of the files to keep only one line per timestamp
    print("\n=== CLEANING FILES ===")
    
    # Columns to remove because they depend on the device
    cols_to_drop = ['home_id', 'device_id', 'device_type', 'room', 'status', 'power_watt', 'indoor_temp']
    
    cleaned_files = []
    for file in temp_files:
        if os.path.exists(file):
            # Reading
            df = pd.read_csv(file, sep=',')
            
            # Keep only the first occurrence of each timestamp
            df_cleaned = df.drop_duplicates(subset=['timestamp'])
            
            # Remove useless columns for your service
            df_cleaned = df_cleaned.drop(columns=cols_to_drop)
            
            # Save
            new_name = f"data_{file}"
            df_cleaned.to_csv(new_name, index=False, sep=';')
            cleaned_files.append(new_name)
            
            print(f"✅ {file} processed -> {new_name}")
            print(f"   Old nb rows: {len(df)} | New nb rows: {len(df_cleaned)}")
    
    return cleaned_files


def get_user_presence(row):
    """
    Determines user presence according to day and hour
    - Monday to Friday (0-4): 9am-5pm away, otherwise at_home
    - Saturday (5): 2pm-6pm away, otherwise at_home
    - Sunday (6): always at_home
    """
    day_of_week = row['day_of_week']
    hour = row['hour_of_day']
    
    if day_of_week <= 4:  # Monday to Friday
        if 9 <= hour < 17:
            return '0'
        else:
            return '1'
    elif day_of_week == 5:  # Saturday
        if 14 <= hour < 18:
            return '0'
        else:
            return '1'
    else:  # Sunday
        return '1'


def add_user_presence(input_path, verbose=True):
    """
    Step 2: Add the user_present column to a CSV file
    """
    if not os.path.exists(input_path):
        print(f"Error: File {input_path} does not exist.")
        return None
    
    # Read the CSV file
    df = pd.read_csv(input_path, sep=';')
    
    # Apply the function to create the user_present column
    df['user_present'] = df.apply(get_user_presence, axis=1)
    
    if verbose:
        # Check the result
        print(f"\nDataset preview with user_present for {input_path}:")
        print(df[['timestamp', 'day_of_week', 'hour_of_day', 'user_present']].head(10))
    
    # Save to a new file
    output_file = input_path.replace('.csv', '_scheduled.csv')
    df.to_csv(output_file, sep=';', index=False)
    
    if verbose:
        print(f"✓ File saved: {output_file}")
    
    return output_file


def add_gps_data(input_path, use_noise=False, noise_intensity=0.33, verbose=True):
    """
    Step 3: Add GPS location data (distance and velocity) based on user presence
    """
    if not os.path.exists(input_path):
        print(f"Error: File {input_path} does not exist.")
        return None

    df = pd.read_csv(input_path, sep=';')
    n_rows = len(df)
    
    distances = np.zeros(n_rows)
    velocities = np.zeros(n_rows)
    
    # Identification of planning blocks (0=Absent, 1=Present)
    df['block'] = (df['user_present'] != df['user_present'].shift()).cumsum()
    
    for _, block_df in df.groupby('block'):
        if block_df['user_present'].iloc[0] == 0:
            indices = block_df.index.tolist()
            
            start_offset = 0
            end_offset = 0

            if use_noise and random.random() < noise_intensity:
                # Simulation of significant offsets: 1h to 3h (4 to 12 rows)
                # Randomly choose an advance or massive delay
                magnitude = random.randint(4, 12) 
                direction = random.choice([-1, 1])
                
                # Apply this either to departure or arrival
                if random.choice(['start', 'end']) == 'start':
                    start_offset = magnitude * direction
                else:
                    end_offset = magnitude * direction
                
                if verbose:
                    print(f"Applied offset of {magnitude} rows in direction {direction} at index {indices[0]}")
            
            # Calculation of real movement points
            real_start = max(0, indices[0] + start_offset)
            real_end = min(n_rows - 1, indices[-1] + end_offset)
            
            real_duration = real_end - real_start
            if real_duration <= 0: continue
            
            # Travel duration: ~20-30 minutes each way (depending on total absence)
            # Minimum 2 rows (30 min), maximum 25% of total duration
            travel_rows = max(2, min(int(real_duration * 0.25), 8))
            
            # Max speed during travel (m/s) - adapted to duration
            max_speed = min(25.0, 5.0 + (real_duration * 0.15))
            
            # Calculate maximum distance reached (destination)
            # Average speed during travel * time per row (900s)
            max_distance = round((max_speed * 0.7) * travel_rows * 900, 2)
            
            # Generation of physical movement: Go -> Stay -> Return
            for i in range(real_duration + 1):
                curr_idx = real_start + i
                if curr_idx >= n_rows: break
                
                if i < travel_rows:
                    # OUTBOUND TRAVEL: Going to destination
                    progress = i / travel_rows  # 0 to 1
                    # Smooth acceleration/deceleration using sine
                    vel = max_speed * np.sin(progress * np.pi / 2)
                    velocities[curr_idx] = round(vel, 2)
                    
                    # Distance increases
                    if i == 0:
                        distances[curr_idx] = round(vel * 900, 2)
                    else:
                        distances[curr_idx] = round(distances[curr_idx-1] + vel * 900, 2)
                
                elif i >= real_duration - travel_rows:
                    # RETURN TRAVEL: Coming back home
                    # Check if already at home (distance is 0)
                    if curr_idx > 0 and distances[curr_idx-1] == 0:
                        velocities[curr_idx] = 0.0
                        distances[curr_idx] = 0.0
                    else:
                        progress = (real_duration - i) / travel_rows  # 1 to 0
                        vel = max_speed * np.sin(progress * np.pi / 2)
                        
                        # Distance decreases back to 0
                        new_distance = round(max(0.0, distances[curr_idx-1] - vel * 900), 2)
                        distances[curr_idx] = new_distance
                        
                        # Velocity is 0 if we've reached home
                        velocities[curr_idx] = 0.0 if new_distance == 0 else round(vel, 2)
                
                else:
                    # STAYING AT DESTINATION: No movement
                    velocities[curr_idx] = 0.0
                    distances[curr_idx] = distances[curr_idx-1]  # Distance stays constant

    df['user_distance'] = distances
    df['user_velocity'] = velocities
    
    df = df.drop(columns=['block'])
    output_path = input_path.replace('.csv', '_GPS.csv')
    df.to_csv(output_path, index=False, sep=';')
    
    if verbose:
        mode = f"RANDOM MODE (Intensity: {noise_intensity})" if use_noise else "SYNCHRONIZED MODE"
        print(f"✅ {mode}: File generated -> {output_path}")
    
    return output_path


def process_all_pipeline(source_file='smartHomeEnergyManagement.csv', add_presence=True, 
                         add_gps=True, use_noise=False, noise_intensity=0.15, verbose=False):
    """
    Complete pipeline: split, clean, add presence, and add GPS data
    """
    print("\n" + "="*60)
    print("SMART HOME DATA PROCESSING PIPELINE")
    print("="*60)
    
    # Step 1: Split and clean
    cleaned_files = split_and_clean_dataset(source_file)
    
    if not add_presence and not add_gps:
        print("\n✅ Pipeline completed: Only split and clean")
        return cleaned_files
    
    # Step 2: Add user presence
    if add_presence:
        print("\n=== STEP 2: ADDING USER PRESENCE ===")
        scheduled_files = []
        for file in cleaned_files:
            result = add_user_presence(file, verbose=verbose)
            if result:
                scheduled_files.append(result)
                if not verbose:
                    print(f"✅ User presence added: {result}")
    else:
        scheduled_files = cleaned_files
    
    if not add_gps:
        print("\n✅ Pipeline completed: Split, clean, and presence added")
        return scheduled_files
    
    # Step 3: Add GPS data
    if add_gps:
        print("\n=== STEP 3: ADDING GPS DATA ===")
        gps_files = []
        for file in scheduled_files:
            result = add_gps_data(file, use_noise=use_noise, 
                                 noise_intensity=noise_intensity, verbose=verbose)
            if result:
                gps_files.append(result)
                if not verbose:
                    mode = f"with noise (intensity={noise_intensity})" if use_noise else "synchronized"
                    print(f"✅ GPS data added ({mode}): {result}")
    
    print("\n" + "="*60)
    print("✅ PIPELINE COMPLETED SUCCESSFULLY")
    print("="*60)
    
    return gps_files if add_gps else scheduled_files


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Process smart home energy data: split, clean, add user presence and GPS data",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Full pipeline with all steps
  python process_smart_home_data.py --full
  
  # Full pipeline with GPS noise and verbose output
  python process_smart_home_data.py --full --noise --level 0.2 --verbose
  
  # Only split and clean
  python process_smart_home_data.py --split-only
  
  # Add user presence to a specific file
  python process_smart_home_data.py --add-presence data_home_1.csv --verbose
  
  # Add GPS data to a specific file
  python process_smart_home_data.py --add-gps data_home_1_scheduled.csv --noise --verbose
        """
    )
    
    # Main modes
    parser.add_argument("--full", action="store_true", 
                       help="Run the full pipeline (split, clean, add presence, add GPS)")
    parser.add_argument("--split-only", action="store_true",
                       help="Only split and clean the dataset")
    
    # Individual operations
    parser.add_argument("--add-presence", type=str, metavar="FILE",
                       help="Add user_present column to a specific file")
    parser.add_argument("--add-gps", type=str, metavar="FILE",
                       help="Add GPS data to a specific file (requires user_present column)")
    
    # Options
    parser.add_argument("--source", type=str, default="smartHomeEnergyManagement.csv",
                       help="Source CSV file for splitting (default: smartHomeEnergyManagement.csv)")
    parser.add_argument("--noise", action="store_true",
                       help="Enable random GPS offsets (2-3h disruptions)")
    parser.add_argument("--level", type=float, default=0.33,
                       help="Noise intensity - probability that a trip is disrupted (0.0 to 1.0, default: 0.33)")
    parser.add_argument("--verbose", action="store_true",
                       help="Enable verbose output with detailed information")
    
    args = parser.parse_args()
    
    # Execute based on arguments
    if args.full:
        # Full pipeline
        process_all_pipeline(
            source_file=args.source,
            add_presence=True,
            add_gps=True,
            use_noise=args.noise,
            noise_intensity=args.level,
            verbose=args.verbose
        )
    
    elif args.split_only:
        # Only split and clean
        process_all_pipeline(
            source_file=args.source,
            add_presence=False,
            add_gps=False
        )
    
    elif args.add_presence:
        # Add presence to specific file
        print("\n=== ADDING USER PRESENCE ===")
        result = add_user_presence(args.add_presence, verbose=args.verbose)
        if result:
            print("\nPresence profiles summary:")
            print("Monday-Friday (day_of_week 0-4): 9am-5pm: away (0) | Other: at_home (1)")
            print("Saturday (day_of_week 5): 2pm-6pm: away (0) | Other: at_home (1)")
            print("Sunday (day_of_week 6): Always: at_home (1)")
    
    elif args.add_gps:
        # Add GPS to specific file
        print("\n=== ADDING GPS DATA ===")
        add_gps_data(args.add_gps, use_noise=args.noise, 
                    noise_intensity=args.level, verbose=args.verbose)
    
    else:
        # No valid option provided
        parser.print_help()
        print("\n❌ Error: Please specify an operation mode (--full, --split-only, --add-presence, or --add-gps)")
