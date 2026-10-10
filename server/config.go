package server

import (
	"encoding/json"
	"fmt"
	"os"

	"go.bug.st/serial"
)

type PlantConfig struct {
	Moisture struct {
	}
}

// Function to load the general program JSON configuration into the serial mode struct
func LoadProgramConfig(filename string) (*serial.Mode, error) {
	file, err := os.Open(filename)

	if err != nil {
		return nil, fmt.Errorf("Failed to open config file: %w", err)
	}
	defer file.Close()

	var cfg serial.Mode

	decoder := json.NewDecoder(file)
	if err := decoder.Decode(&cfg); err != nil {
		return nil, fmt.Errorf("Failed to decode JSON config: %w", err)
	}

	return &cfg, nil
}

// Function to load the plant JSON configuration into the plant config struct
func LoadPlantConfig(filename string) (*PlantConfig, error) {
	file, err := os.Open(filename)

	if err != nil {
		return nil, fmt.Errorf("Failed to open config file: %w", err)
	}
	defer file.Close()

	var cfg PlantConfig

	decoder := json.NewDecoder(file)
	if err := decoder.Decode(&cfg); err != nil {
		return nil, fmt.Errorf("Failed to decode JSON config: %w", err)
	}

	return &cfg, nil
}

func CheckPlantThresholds() {

}
