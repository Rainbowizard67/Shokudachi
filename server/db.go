package server

import (
	"database/sql"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func OpenDbFile() *sql.DB {
	db, err := sql.Open("sqlite3", "./shoku.db")
	if err != nil {
		log.Fatal(err)
	}

	return db
}

func InitDbFile() {
	db, err := sql.Open("sqlite3", "./shoku.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	sql_create_users := `CREATE TABLE IF NOT EXISTS users (
		user_id INTEGER PRIMARY KEY NOT NULL,
		user TEXT NOT NULL UNIQUE,
		pin INTEGER CHECK (pin >= 1000 AND pin <= 9999)
	);`

	//12 bit analog to digital convert, 0 to 4095
	sql_create_data := `CREATE TABLE IF NOT EXISTS data (
		plant_id INTEGER PRIMARY KEY NOT NULL,
		user_id INTEGER NOT NULL REFERENCES users(user_id),
		plant_moisture_volt INTEGER NOT NULL
	);`

	if _, err := db.Exec(sql_create_users); err != nil {
		log.Fatal("Create users table: ", err)
	}

	if _, err := db.Exec(sql_create_data); err != nil {
		log.Fatal("Create data table: ", err)
	}

	log.Println("Database tables initialized")
}
