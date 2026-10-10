package server

import (
	"encoding/json"
	"fmt"
	"os"
)

type ProgramConfig struct {
	Baud_Rate int `json:"baud_rate"`
}

type PlantConfig struct {
	Nitrogen struct {
	}
	Phosphorus struct {
	}
	Potassium struct {
	}
	Moisture struct {
	}
}

// Function to load the general program JSON configuration into the program struct
func LoadProgramConfig(filename string) (*ProgramConfig, error) {
	file, err := os.Open(filename)

	if err != nil {
		return nil, fmt.Errorf("Failed to open config file: %w", err)
	}
	defer file.Close()

	var cfg ProgramConfig

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
