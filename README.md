# Billing & Inventory Management System in C

A command-line **billing and inventory management system written in C**, designed to manage products, stock, shopping carts, customers, VAT rates and invoices.

The project focuses on implementing a complete data-management system using low-level programming concepts such as **manual memory management, hash tables, linked lists, dynamic arrays, sorting algorithms and input parsing**.

## Overview

The application simulates the core operations of a retail billing system.

It supports:

* product registration and updates
* stock management
* EAN-8 and EAN-13 validation
* configurable VAT rates
* shopping cart operations
* invoice generation
* customer information
* invoice search and deletion
* wildcard product searches
* global sales statistics

The system is controlled entirely through commands received from standard input.

---

## Main Features

### Product Management

Products are identified through **EAN-8 or EAN-13 codes**.

Each product stores:

* EAN code
* VAT category
* unit price
* current stock
* units sold
* units currently in the shopping cart
* description
* active/inactive state

Product input is validated before being stored.

The system checks:

* EAN validity and control digit
* VAT category
* positive prices
* valid quantities
* product description format

Existing products can also be updated.

---

### Hash-Based Product Lookup

Products are indexed through a custom **hash table** to provide efficient lookups by EAN.

The hash table contains **10,007 buckets** and uses a polynomial hash function.

Collisions are handled using **separate chaining with linked lists**.

```text
EAN
 │
 ▼
Hash Function
 │
 ▼
+---------+
| Bucket  |
+---------+
     │
     ▼
 Product ──> Product ──> Product
```

This allows product retrieval without scanning the entire product array.

---

### Shopping Cart

The system includes a shopping cart where products can be added or removed before an invoice is generated.

When products are added to the cart:

* stock is decreased
* cart quantity is increased
* availability is checked
* VAT-inclusive totals are calculated

Removing items from the cart restores the corresponding stock.

The cart can also be listed in EAN order.

---

### VAT Calculation

Products are associated with VAT categories identified by letters.

Default VAT categories include:

```text
A -> 0%
B -> 6%
C -> 13%
D -> 23%
```

The VAT table can also be loaded from an external configuration file.

Product totals are calculated using:

```text
total = quantity × unit_price × (1 + VAT / 100)
```

Values are rounded to two decimal places.

---

### Invoice Management

The application can generate invoices directly from the contents of the shopping cart.

Each invoice stores:

* invoice number
* customer tax identification number
* customer name
* number of purchased items
* total amount
* active/inactive state

When an invoice is created:

```text
Shopping Cart
     │
     ▼
Calculate Total + VAT
     │
     ▼
Create Invoice
     │
     ▼
Update Sales Statistics
     │
     ▼
Clear Shopping Cart
```

Invoices are stored in a **dynamically allocated array** that automatically grows when more capacity is required.

---

### Customer Management

Invoices can optionally include customer information.

The system validates:

* Portuguese-style 9-digit tax identification numbers
* customer names
* quoted names containing spaces

If no customer is specified, a default final-consumer customer is used.

---

### Product Search with Wildcards

Product listings support wildcard patterns.

Two wildcard symbols are implemented:

```text
*  -> matches any sequence of characters
?  -> matches exactly one character
```

For example:

```text
560*
```

matches every available EAN beginning with `560`.

The wildcard matcher is implemented recursively.

---

### Sorting

The system implements its own **Merge Sort** algorithm.

Sorting is performed indirectly using arrays of indexes rather than moving the complete structures.

Products can be ordered by:

```text
EAN code
```

Invoices can be ordered by:

```text
Customer name
      │
      └── if equal -> Invoice number
```

Merge Sort provides:

```text
Time Complexity: O(n log n)
```

---

## Commands

The program is controlled through single-character commands.

| Command | Function                                                    |
| ------- | ----------------------------------------------------------- |
| `p`     | Create or update a product                                  |
| `l`     | List available products                                     |
| `a`     | Add/remove products from the shopping cart or list the cart |
| `r`     | Display system statistics or product information            |
| `f`     | Generate an invoice                                         |
| `c`     | List invoices                                               |
| `d`     | Delete an invoice or remove product stock                   |
| `q`     | Exit the program                                            |

### Example

```text
p <ean> <vat> <price> <quantity> <description>
```

Adds a new product or updates an existing one.

```text
a <ean>
```

Adds one unit of a product to the shopping cart.

```text
a <quantity> <ean>
```

Changes the quantity of a product in the cart.

```text
f <nif> <customer-name>
```

Generates an invoice using the current shopping cart.

---

## Data Structures

The project uses several custom structures.

### Product

```c
typedef struct {
    char ean[14];
    char iva;
    double preco;
    int stock;
    int vendidos;
    int cesto;
    char descricao[51];
    int ativo;
    int em_lista_cesto;
} Produto;
```

### Invoice

```c
typedef struct {
    int numero;
    int nif;
    char *nome;
    int n_items;
    double total;
    int ativa;
} Factura;
```

### Hash Table Node

```c
typedef struct NoHashProd {
    char ean[14];
    int indice_produto;
    struct NoHashProd *prox;
} NoHashProd;
```

### Main System

The main system structure combines:

```text
Products
Invoices
VAT table
Product hash table
Shopping cart
Sales statistics
```

into a single central data structure.

---

## Memory Management

The project uses manual dynamic memory management.

Dynamic memory is used for:

* invoice storage
* customer names
* hash table linked-list nodes

The invoice array automatically expands using `realloc()` when necessary.

Before terminating, the application releases all dynamically allocated memory.

This includes:

```text
Customer names
Hash-table nodes
Invoice array
```

---

## Architecture

The implementation is divided into modules according to responsibility:

```text
.
├── project.c
├── sistema.h
├── base.c
├── produtos.c
├── cesto.c
├── facturas.c
└── ordenacao.c
```

### `project.c`

Main application entry point.

Responsible for:

* initializing the system
* loading VAT configuration
* reading commands
* dispatching operations
* releasing memory before termination

### `sistema.h`

Contains:

* constants
* data structures
* error messages
* function prototypes

### `base.c`

Provides core utilities including:

* initialization
* VAT configuration
* hash functions
* hash-table operations
* string validation
* memory cleanup

### `produtos.c`

Implements:

* EAN validation
* product creation
* product updates
* product search and listing
* wildcard matching
* sales summaries

### `cesto.c`

Implements:

* shopping-cart operations
* stock transfers
* VAT calculations
* monetary rounding
* cart listing

### `facturas.c`

Implements:

* customer validation
* invoice creation
* invoice lookup
* invoice listing
* invoice removal
* stock-removal operations
* dynamic invoice storage

### `ordenacao.c`

Implements a reusable **Merge Sort** algorithm for products and invoices.

---

## Technologies & Concepts

* **C**
* Data Structures
* Hash Tables
* Linked Lists
* Dynamic Arrays
* Pointers
* Dynamic Memory Allocation
* Merge Sort
* Recursion
* String Processing
* File I/O
* Input Parsing
* Modular Programming
* Algorithm Design
* Inventory Management
* Billing Systems

---

## Key Technical Highlights

Some of the main technical aspects of the project include:

* custom EAN-8/EAN-13 validation
* hash table with separate chaining
* O(n log n) Merge Sort implementation
* recursive wildcard matching
* dynamic invoice storage using `realloc`
* manual memory management
* VAT-aware price calculations
* modular multi-file C architecture
* validation of complex command-line input
* consistent error handling
* stock and shopping-cart synchronization

---

## Program Flow

```text
                 ┌───────────────┐
                 │   Start       │
                 └───────┬───────┘
                         │
                         ▼
                Initialize System
                         │
                         ▼
                Load VAT Configuration
                         │
                         ▼
                  Read Command
                         │
        ┌────────────────┼────────────────┐
        │                │                │
        ▼                ▼                ▼
     Products          Cart           Invoices
        │                │                │
        └────────────────┼────────────────┘
                         │
                         ▼
                    Update State
                         │
                         ▼
                   Next Command
                         │
                         ▼
                        `q`
                         │
                         ▼
                   Free Memory
                         │
                         ▼
                       Exit
```

---

## Academic Context

Developed as part of an **Introduction to Algorithms and Data Structures** programming project at Instituto Superior Técnico.

The project focuses on applying fundamental C programming concepts to a larger modular system involving data structures, algorithms, memory management and efficient data retrieval.

