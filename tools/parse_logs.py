#!/usr/bin/env python3
"""
Battery Log Parser & Analyzer for DIY Charger Simple

This script parses CSV logs from the charger and generates:
- Discharge/charge curves
- Capacity statistics
- Battery health analysis
- Comparison charts

Usage:
    python parse_logs.py battery_log.csv
    python parse_logs.py battery_log.csv --plot --export
"""

import sys
import csv
import argparse
from datetime import datetime
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import numpy as np
    PLOTTING_AVAILABLE = True
except ImportError:
    PLOTTING_AVAILABLE = False
    print("Warning: matplotlib not installed. Plotting disabled.")
    print("Install with: pip install matplotlib numpy")

class BatteryLog:
    """Represents a single battery test log"""
    
    def __init__(self, filename):
        self.filename = filename
        self.data = []
        self.metadata = {
            'port': None,
            'battery_type': None,
            'mode': None,
            'start_time': None,
            'end_time': None,
            'duration_seconds': 0,
            'final_capacity_mah': 0,
            'final_energy_wh': 0,
            'avg_voltage': 0,
            'avg_current': 0
        }
        
    def load(self):
        """Load CSV file and parse data"""
        with open(self.filename, 'r') as f:
            reader = csv.DictReader(f)
            
            for row in reader:
                try:
                    data_point = {
                        'timestamp': int(row['Timestamp']),
                        'port': int(row['Port']),
                        'voltage': float(row['Voltage(V)']),
                        'current': float(row['Current(A)']),
                        'power': float(row['Power(W)']),
                        'mah': float(row['mAh']),
                        'wh': float(row['Wh']),
                        'mode': row['Mode'],
                        'battery': row['Battery'],
                        'status': row['Status']
                    }
                    self.data.append(data_point)
                except (ValueError, KeyError) as e:
                    print(f"Warning: Skipping invalid row: {e}")
                    continue
        
        if not self.data:
            raise ValueError("No valid data found in CSV file")
        
        # Calculate metadata
        self._calculate_metadata()
        
    def _calculate_metadata(self):
        """Calculate summary statistics"""
        if not self.data:
            return
        
        self.metadata['port'] = self.data[0]['port']
        self.metadata['battery_type'] = self.data[0]['battery']
        self.metadata['mode'] = self.data[0]['mode']
        self.metadata['start_time'] = self.data[0]['timestamp']
        self.metadata['end_time'] = self.data[-1]['timestamp']
        self.metadata['duration_seconds'] = self.metadata['end_time'] - self.metadata['start_time']
        self.metadata['final_capacity_mah'] = self.data[-1]['mah']
        self.metadata['final_energy_wh'] = self.data[-1]['wh']
        
        voltages = [d['voltage'] for d in self.data]
        currents = [d['current'] for d in self.data]
        
        self.metadata['avg_voltage'] = sum(voltages) / len(voltages)
        self.metadata['avg_current'] = sum(currents) / len(currents)
        self.metadata['min_voltage'] = min(voltages)
        self.metadata['max_voltage'] = max(voltages)
        
    def print_summary(self):
        """Print summary statistics"""
        print("\n" + "="*60)
        print(f"Battery Test Summary - {Path(self.filename).name}")
        print("="*60)
        print(f"Port:          {self.metadata['port'] + 1}")
        print(f"Battery Type:  {self.metadata['battery_type']}")
        print(f"Mode:          {self.metadata['mode']}")
        print(f"Duration:      {self.metadata['duration_seconds']} seconds ({self.metadata['duration_seconds']/3600:.2f} hours)")
        print(f"\nCapacity:      {self.metadata['final_capacity_mah']:.1f} mAh")
        print(f"Energy:        {self.metadata['final_energy_wh']:.2f} Wh")
        print(f"\nVoltage Range: {self.metadata['min_voltage']:.3f}V - {self.metadata['max_voltage']:.3f}V")
        print(f"Avg Voltage:   {self.metadata['avg_voltage']:.3f}V")
        print(f"Avg Current:   {self.metadata['avg_current']:.3f}A")
        print(f"\nData Points:   {len(self.data)}")
        print("="*60 + "\n")
        
    def plot_curves(self, save_path=None):
        """Generate discharge/charge curves"""
        if not PLOTTING_AVAILABLE:
            print("Plotting not available. Install matplotlib.")
            return
        
        timestamps = [d['timestamp'] for d in self.data]
        voltages = [d['voltage'] for d in self.data]
        currents = [d['current'] for d in self.data]
        capacities = [d['mah'] for d in self.data]
        powers = [d['power'] for d in self.data]
        
        # Convert timestamps to hours
        time_hours = [(t - timestamps[0]) / 3600 for t in timestamps]
        
        # Create figure with subplots
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        fig.suptitle(f'Battery Test Analysis - Port {self.metadata["port"] + 1} ({self.metadata["battery_type"]})', 
                     fontsize=14, fontweight='bold')
        
        # Voltage vs Time
        axes[0, 0].plot(time_hours, voltages, 'b-', linewidth=2)
        axes[0, 0].set_xlabel('Time (hours)')
        axes[0, 0].set_ylabel('Voltage (V)')
        axes[0, 0].set_title('Voltage vs Time')
        axes[0, 0].grid(True, alpha=0.3)
        
        # Current vs Time
        axes[0, 1].plot(time_hours, currents, 'r-', linewidth=2)
        axes[0, 1].set_xlabel('Time (hours)')
        axes[0, 1].set_ylabel('Current (A)')
        axes[0, 1].set_title('Current vs Time')
        axes[0, 1].grid(True, alpha=0.3)
        
        # Capacity vs Time
        axes[1, 0].plot(time_hours, capacities, 'g-', linewidth=2)
        axes[1, 0].set_xlabel('Time (hours)')
        axes[1, 0].set_ylabel('Capacity (mAh)')
        axes[1, 0].set_title(f'Capacity Accumulation (Final: {capacities[-1]:.1f} mAh)')
        axes[1, 0].grid(True, alpha=0.3)
        
        # Voltage vs Capacity
        axes[1, 1].plot(capacities, voltages, 'm-', linewidth=2)
        axes[1, 1].set_xlabel('Capacity (mAh)')
        axes[1, 1].set_ylabel('Voltage (V)')
        axes[1, 1].set_title('Discharge/Charge Curve')
        axes[1, 1].grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=300, bbox_inches='tight')
            print(f"Plot saved to: {save_path}")
        else:
            plt.show()
            
    def export_summary(self, output_file):
        """Export summary to text file"""
        with open(output_file, 'w') as f:
            f.write("="*60 + "\n")
            f.write(f"Battery Test Summary - {Path(self.filename).name}\n")
            f.write("="*60 + "\n")
            f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            f.write("TEST INFORMATION\n")
            f.write("-" * 60 + "\n")
            f.write(f"Port:          {self.metadata['port'] + 1}\n")
            f.write(f"Battery Type:  {self.metadata['battery_type']}\n")
            f.write(f"Mode:          {self.metadata['mode']}\n")
            f.write(f"Duration:      {self.metadata['duration_seconds']} seconds ({self.metadata['duration_seconds']/3600:.2f} hours)\n\n")
            
            f.write("RESULTS\n")
            f.write("-" * 60 + "\n")
            f.write(f"Final Capacity:  {self.metadata['final_capacity_mah']:.1f} mAh\n")
            f.write(f"Final Energy:    {self.metadata['final_energy_wh']:.2f} Wh\n\n")
            
            f.write("VOLTAGE STATISTICS\n")
            f.write("-" * 60 + "\n")
            f.write(f"Minimum:       {self.metadata['min_voltage']:.3f}V\n")
            f.write(f"Maximum:       {self.metadata['max_voltage']:.3f}V\n")
            f.write(f"Average:       {self.metadata['avg_voltage']:.3f}V\n")
            f.write(f"Range:         {self.metadata['max_voltage'] - self.metadata['min_voltage']:.3f}V\n\n")
            
            f.write("CURRENT STATISTICS\n")
            f.write("-" * 60 + "\n")
            f.write(f"Average:       {self.metadata['avg_current']:.3f}A\n\n")
            
            f.write("DATA QUALITY\n")
            f.write("-" * 60 + "\n")
            f.write(f"Data Points:   {len(self.data)}\n")
            f.write(f"Sample Rate:   {len(self.data) / (self.metadata['duration_seconds']/60):.2f} samples/minute\n\n")
            
            # Battery health estimation
            f.write("BATTERY HEALTH ESTIMATION\n")
            f.write("-" * 60 + "\n")
            
            if self.metadata['battery_type'] == 'Li-ion':
                rated_capacity = 2500  # Typical 18650
                health_percent = (self.metadata['final_capacity_mah'] / rated_capacity) * 100
                f.write(f"Assumed rated capacity: {rated_capacity} mAh\n")
                f.write(f"Measured capacity:      {self.metadata['final_capacity_mah']:.1f} mAh\n")
                f.write(f"Health estimation:      {health_percent:.1f}%\n\n")
                
                if health_percent >= 80:
                    f.write("Status: GOOD - Battery in good condition\n")
                elif health_percent >= 60:
                    f.write("Status: FAIR - Battery showing age, usable\n")
                else:
                    f.write("Status: POOR - Battery degraded, consider replacement\n")
            
            f.write("\n" + "="*60 + "\n")
        
        print(f"Summary exported to: {output_file}")

def main():
    parser = argparse.ArgumentParser(
        description='Parse and analyze battery test logs from DIY Charger Simple'
    )
    parser.add_argument('input_file', help='CSV log file to analyze')
    parser.add_argument('--plot', action='store_true', help='Generate plots')
    parser.add_argument('--export', action='store_true', help='Export summary to text file')
    parser.add_argument('--save-plot', metavar='FILE', help='Save plot to file instead of displaying')
    
    args = parser.parse_args()
    
    # Check if file exists
    if not Path(args.input_file).exists():
        print(f"Error: File '{args.input_file}' not found")
        sys.exit(1)
    
    # Load and analyze log
    print(f"Loading log file: {args.input_file}")
    log = BatteryLog(args.input_file)
    
    try:
        log.load()
    except Exception as e:
        print(f"Error loading file: {e}")
        sys.exit(1)
    
    # Print summary
    log.print_summary()
    
    # Export summary if requested
    if args.export:
        base_name = Path(args.input_file).stem
        output_file = f"{base_name}_summary.txt"
        log.export_summary(output_file)
    
    # Generate plots if requested
    if args.plot or args.save_plot:
        if not PLOTTING_AVAILABLE:
            print("Error: matplotlib not installed")
            print("Install with: pip install matplotlib numpy")
            sys.exit(1)
        
        save_path = args.save_plot if args.save_plot else None
        if args.plot and not save_path:
            save_path = None  # Will show interactive plot
        elif not save_path:
            base_name = Path(args.input_file).stem
            save_path = f"{base_name}_plot.png"
        
        log.plot_curves(save_path)

if __name__ == '__main__':
    main()

"""
Example usage:

# Basic analysis
python parse_logs.py battery_log_port1.csv

# Generate plots
python parse_logs.py battery_log_port1.csv --plot

# Export summary and save plot
python parse_logs.py battery_log_port1.csv --export --save-plot output.png

# Batch process multiple files
for file in *.csv; do
    python parse_logs.py "$file" --export --save-plot "${file%.csv}_plot.png"
done
"""
