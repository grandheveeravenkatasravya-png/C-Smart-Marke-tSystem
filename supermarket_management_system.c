// ...existing code...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PRODUCTS 2000
#define STRLEN 128
#define DBFILE "product_db.csv"
#define WORKERS_FILE "workers.txt"
#define META_FILE "supermarket.txt"
#define TEXT_EXPORT_FILE "product_records.txt"

typedef struct {
    int serial;
    char prod_id[32];
    char name[128];
    char category[64];
    double price;
    char mfg[16];
    char exp[16];
    char brand[64];
    int stock;
    char worker_name[64];
    char worker_phone[32];
    char worker_shift[32];
    int sold_today;
    double discount; // percent e.g. 10.0 for 10%
} Product;

typedef struct {
    char name[64];
    char phone[32];
    char shift[32];
    char category[64];
} Worker;

Product products[MAX_PRODUCTS];
int product_count = 0;

Worker workers[5] = {
    {"Ravi Teja","9876500001","Morning","Dairy"},
    {"Siva Kartheyan","9876500002","Morning","Bakery"},
    {"Rajesh Gurunadam","9876500003","Afternoon","Beverages"},
    {"Anita Sharma","9876500004","Evening","Snacks"},
    {"Pooja Pandya","9876500005","Night","Household"}
};

const char *owner_name = "Veera Venkata Sravya Grandhe";
const char *owner_phone = "6304651563";
const char *super_name = "Sravya Elite Mart";
const char *timings = "08:00 AM - 10:00 PM";

/* Utility: trim newline */
static void trimnl(char *s){ char *p=strchr(s,'\n'); if(p)*p=0; }

/* Save products to CSV (full record incl sold & discount) */
void save_db(void){
    FILE *f = fopen(DBFILE,"w");
    if(!f){ perror("save_db fopen"); return; }
    fprintf(f,"serial,prod_id,name,category,price,mfg,exp,brand,stock,worker_name,worker_phone,worker_shift,sold_today,discount\n");
    for(int i=0;i<product_count;i++){
        Product *p=&products[i];
        // ensure no commas in data (simple replace)
        char name[128]; strncpy(name,p->name,sizeof(name)); name[sizeof(name)-1]=0; for(char *c=name;*c;c++) if(*c==',') *c=' ';
        char brand[64]; strncpy(brand,p->brand,sizeof(brand)); brand[sizeof(brand)-1]=0; for(char *c=brand;*c;c++) if(*c==',') *c=' ';
        fprintf(f,"%d,%s,%s,%s,%.2f,%s,%s,%s,%d,%s,%s,%s,%d,%.2f\n",
            p->serial,p->prod_id,name,p->category,p->price,p->mfg,p->exp,brand,p->stock,
            p->worker_name,p->worker_phone,p->worker_shift,p->sold_today,p->discount);
    }
    fclose(f);
}

/* Save workers and meta info for convenience */
void save_meta(void){
    FILE *f = fopen(WORKERS_FILE,"w");
    if(f){
        for(int i=0;i<5;i++){
            fprintf(f,"%s,%s,%s,%s\n",workers[i].name,workers[i].phone,workers[i].shift,workers[i].category);
        }
        fclose(f);
    }
    f = fopen(META_FILE,"w");
    if(f){
        fprintf(f,"Supermarket:%s\nOwner:%s\nContact:%s\nTimings:%s\n",super_name,owner_name,owner_phone,timings);
        fclose(f);
    }
}

/* Load DB if exists; returns 1 if loaded, 0 otherwise */
int load_db(void){
    FILE *f = fopen(DBFILE,"r");
    if(!f) return 0;
    char line[1024];
    // skip header
    if(!fgets(line,sizeof(line),f)){ fclose(f); return 0; }
    product_count = 0;
    while(fgets(line,sizeof(line),f)){
        trimnl(line);
        if(strlen(line)==0) continue;
        char *tok;
        Product p; memset(&p,0,sizeof(p));
        tok = strtok(line,","); if(!tok) continue; p.serial = atoi(tok);
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.prod_id,tok,sizeof(p.prod_id));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.name,tok,sizeof(p.name));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.category,tok,sizeof(p.category));
        tok = strtok(NULL,","); if(!tok) continue; p.price = atof(tok);
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.mfg,tok,sizeof(p.mfg));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.exp,tok,sizeof(p.exp));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.brand,tok,sizeof(p.brand));
        tok = strtok(NULL,","); if(!tok) continue; p.stock = atoi(tok);
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.worker_name,tok,sizeof(p.worker_name));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.worker_phone,tok,sizeof(p.worker_phone));
        tok = strtok(NULL,","); if(!tok) continue; strncpy(p.worker_shift,tok,sizeof(p.worker_shift));
        tok = strtok(NULL,","); if(!tok) continue; p.sold_today = atoi(tok);
        tok = strtok(NULL,","); if(!tok) continue; p.discount = atof(tok);
        if(product_count < MAX_PRODUCTS) products[product_count++] = p;
    }
    fclose(f);
    return 1;
}

/* Generate initial 200 products (used if DB missing) */
void generate_initial_db(void){
    const char *categories[] = {"Dairy","Bakery","Beverages","Snacks","Household","PersonalCare","Produce","Frozen","Canned","BabyCare"};
    const char *brands[] = {"FreshFarm","BakeHouse","PureDrinks","Crunchy","HomeCare","NiceSkin","GreenLeaf","Frosty","CanGood","BabyPlus"};
    const char *baseNames[] = {
        "Whole Milk 1L","Skimmed Milk 500ml","Paneer 200g","Cheese Slice 200g","Yogurt Cup 150g",
        "White Bread","Whole Wheat Bread","Bun Pack 6","Croissant","Bagel Pack",
        "Cola 500ml","Orange Juice 1L","Mineral Water 1.5L","Iced Tea 500ml","Energy Drink 330ml",
        "Potato Chips 100g","Salted Peanuts 150g","Chocolate Bar 50g","Cookies Pack 200g","Crackers 150g",
        "Detergent 1kg","Dishwash Liquid 500ml","Toilet Cleaner 750ml","Garbage Bags 20pc","Bleach 1L",
        "Shampoo 200ml","Soap Bar 75g","Toothpaste 100g","Toothbrush Soft","Deodorant 150ml",
        "Banana Bunch","Apple (kg)","Tomato (kg)","Onion (kg)","Potato (kg)",
        "Frozen Peas 500g","Frozen Pizza 350g","Ice Cream 500ml","Frozen Fries 1kg","Fish Fingers 300g",
        "Canned Beans 400g","Canned Corn 400g","Tomato Sauce 500g","Canned Tuna 185g","Canned Soup 400g",
        "Baby Diapers M 30pc","Baby Wet Wipes 80pc","Baby Powder 200g","Baby Shampoo 200ml","Baby Lotion 200ml"
    };
    int nBase = sizeof(baseNames)/sizeof(baseNames[0]);
    int nCat = sizeof(categories)/sizeof(categories[0]);
    int nBrand = sizeof(brands)/sizeof(brands[0]);
    product_count = 0;
    for(int i=0;i<200;i++){
        Product p;
        memset(&p,0,sizeof(p));
        p.serial = i+1;
        snprintf(p.prod_id,sizeof(p.prod_id),"P%04d",1001+i);
        if(i < nBase) strncpy(p.name, baseNames[i], sizeof(p.name));
        else snprintf(p.name,sizeof(p.name),"%s - Var%d", baseNames[i % nBase], (i / nBase)+1);
        strncpy(p.category, categories[i % nCat], sizeof(p.category));
        strncpy(p.brand, brands[i % nBrand], sizeof(p.brand));
        // price pattern
        double price = 20.0 + (i % 100) * 0.5;
        if(strcmp(p.category,"Dairy")==0) price = 40 + (i%30);
        if(strcmp(p.category,"Bakery")==0) price = 25 + (i%20);
        if(strcmp(p.category,"Beverages")==0) price = 30 + (i%40);
        if(strcmp(p.category,"Snacks")==0) price = 15 + (i%30);
        p.price = ((int)(price*100+0.5))/100.0;
        // dates simple
        int d = (i%28)+1;
        int m = ((i/28)%12)+1;
        int y = 2024 + ((i/100)%2);
        snprintf(p.mfg,sizeof(p.mfg),"%04d-%02d-%02d",y,m,d);
        int addMonths = 12;
        if(strcmp(p.category,"Dairy")==0) addMonths = 6;
        if(strcmp(p.category,"Bakery")==0) addMonths = 3;
        if(strcmp(p.category,"Produce")==0) addMonths = 1;
        int expMonth = m + addMonths;
        int expYear = y + (expMonth-1)/12;
        expMonth = ((expMonth-1)%12)+1;
        snprintf(p.exp,sizeof(p.exp),"%04d-%02d-%02d",expYear,expMonth,d);
        p.stock = ((i%60)==0) ? 0 : ((i%60)+1);
        int widx = (i%5);
        strncpy(p.worker_name, workers[widx].name, sizeof(p.worker_name));
        strncpy(p.worker_phone, workers[widx].phone, sizeof(p.worker_phone));
        strncpy(p.worker_shift, workers[widx].shift, sizeof(p.worker_shift));
        p.sold_today = (p.stock>0) ? (i%7) : 0;
        p.discount = (i % 13 == 0) ? 5.0 : 0.0; // some discounts
        products[product_count++] = p;
    }
    save_db();
    save_meta();
}

/* Helpers */
Product* find_by_id(const char *id){
    for(int i=0;i<product_count;i++) if(strcmp(products[i].prod_id,id)==0) return &products[i];
    return NULL;
}
int find_index_by_id(const char *id){
    for(int i=0;i<product_count;i++) if(strcmp(products[i].prod_id,id)==0) return i;
    return -1;
}
void pause_continue_prompt(void){
    char ch[8];
    printf("\nContinue (y/n)? ");
    if(!fgets(ch,sizeof(ch),stdin)) return;
    if(tolower(ch[0])!='y') {
        printf("Exiting.\n");
        exit(0);
    }
}

/* Display functions */
void display_all_products(void){
    printf("SNo | ProdID  | Product Name                      | Category      | Price   | Mfg Date   | Exp Date   | Brand       | Stock | Worker | Phone | Shift\n");
    printf("------------------*---------------------------------------------------*-----------------------------------*--------------------\n");
    for(int i=0;i<product_count;i++){
        Product *p=&products[i];
        printf("%3d | %-7s | %-32s | %-13s | %7.2f | %-10s | %-10s | %-11s | %5d | %-7s | %-10s | %-6s\n",
            p->serial,p->prod_id,p->name,p->category,p->price,p->mfg,p->exp,p->brand,p->stock,
            p->worker_name,p->worker_phone,p->worker_shift);
    }
    printf("Total products: %d\n",product_count);
}

void display_unavailable(void){
    printf("Unavailable products (stock=0):\n");
    printf("SNo | ProdID  | Product Name                      | Category      | Brand\n");
    printf("-----------------------------------------------------------------\n");
    int cnt=0;
    for(int i=0;i<product_count;i++){
        if(products[i].stock==0){
            Product *p=&products[i];
            printf("%3d | %-7s | %-32s | %-13s | %-11s\n", p->serial,p->prod_id,p->name,p->category,p->brand);
            cnt++;
        }
    }
    if(cnt==0) printf("None\n");
}

/* Search helpers (unchanged) */
void search_by_id(void){
    char q[64];
    printf("Enter product id: ");
    if(!fgets(q,sizeof(q),stdin)) return; trimnl(q);
    Product *p = find_by_id(q);
    if(!p) printf("Not found.\n");
    else {
        printf("Found: %s | %s | %s | Price: %.2f | Stock: %d | Worker: %s | Phone: %s | Shift: %s\n",
            p->prod_id,p->name,p->category,p->price,p->stock,p->worker_name,p->worker_phone,p->worker_shift);
    }
}

void search_by_name(void){
    char q[128];
    printf("Enter product name (partial allowed, case-insensitive): ");
    if(!fgets(q,sizeof(q),stdin)) return; trimnl(q);
    // lowercase query
    for(char *c=q;*c;c++) *c = tolower(*c);
    int found=0;
    for(int i=0;i<product_count;i++){
        char namelow[128]; strncpy(namelow,products[i].name,sizeof(namelow)); namelow[sizeof(namelow)-1]=0;
        for(char *c=namelow;*c;c++) *c = tolower(*c);
        if(strstr(namelow,q)){
            Product *p=&products[i];
            printf("%3d | %-7s | %-32s | %-13s | %7.2f | Stock:%d\n",p->serial,p->prod_id,p->name,p->category,p->price,p->stock);
            found++;
        }
    }
    if(!found) printf("No match.\n");
}

/* New: unified search menu */
void search_menu(void){
    char ch[8];
    printf("Search by:\n1) Product ID\n2) Product Name\nChoose (1/2): ");
    if(!fgets(ch,sizeof(ch),stdin)) return;
    int opt = atoi(ch);
    if(opt==1) search_by_id();
    else if(opt==2) search_by_name();
    else printf("Invalid choice.\n");
}

/* Add product */
void add_product(void){
    if(product_count >= MAX_PRODUCTS){ printf("DB full.\n"); return; }
    Product p; memset(&p,0,sizeof(p));
    p.serial = product_count + 1;
    char buf[256];
    printf("Enter product id (e.g. P1201): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.prod_id,buf,sizeof(p.prod_id));
    if(find_by_id(p.prod_id)){ printf("ID exists.\n"); return; }
    printf("Enter product name: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.name,buf,sizeof(p.name));
    printf("Enter category: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.category,buf,sizeof(p.category));
    printf("Enter brand: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.brand,buf,sizeof(p.brand));
    printf("Enter price: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; p.price = atof(buf);
    printf("Enter mfg date (YYYY-MM-DD): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.mfg,buf,sizeof(p.mfg));
    printf("Enter exp date (YYYY-MM-DD): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.exp,buf,sizeof(p.exp));
    printf("Enter stock (integer): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; p.stock = atoi(buf);
    printf("Enter worker name assigned: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.worker_name,buf,sizeof(p.worker_name));
    printf("Enter worker phone: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.worker_phone,buf,sizeof(p.worker_phone));
    printf("Enter worker shift: ");
    if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); strncpy(p.worker_shift,buf,sizeof(p.worker_shift));
    printf("Enter sold today (integer): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; p.sold_today = atoi(buf);
    printf("Enter discount percent (e.g. 5 for 5%%): ");
    if(!fgets(buf,sizeof(buf),stdin)) return; p.discount = atof(buf);
    products[product_count++] = p;
    save_db(); // persist immediately
    printf("Product added and saved.\n");
}

/* Delete product by id */
void delete_product(void){
    char id[64];
    printf("Enter product id to delete: ");
    if(!fgets(id,sizeof(id),stdin)) return; trimnl(id);
    int idx = find_index_by_id(id);
    if(idx==-1){ printf("Not found.\n"); return; }
    // shift left
    for(int i=idx;i<product_count-1;i++){
        products[i] = products[i+1];
        products[i].serial = i+1;
    }
    product_count--;
    // fix serials
    for(int i=0;i<product_count;i++) products[i].serial = i+1;
    save_db();
    printf("Deleted and saved.\n");
}

/* Edit product (choose by id); allows editing most fields */
void edit_product(void){
    char id[64]; char buf[256];
    printf("Enter product id to edit: ");
    if(!fgets(id,sizeof(id),stdin)) return; trimnl(id);
    Product *p = find_by_id(id);
    if(!p){ printf("Not found.\n"); return; }
    printf("Editing %s (%s). Leave line blank to keep current value.\n", p->name, p->prod_id);
    printf("Current name: %s\nNew name: ", p->name); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->name,buf,sizeof(p->name));
    printf("Current category: %s\nNew category: ", p->category); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->category,buf,sizeof(p->category));
    printf("Current brand: %s\nNew brand: ", p->brand); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->brand,buf,sizeof(p->brand));
    printf("Current price: %.2f\nNew price: ", p->price); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) p->price = atof(buf);
    printf("Current mfg: %s\nNew mfg: ", p->mfg); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->mfg,buf,sizeof(p->mfg));
    printf("Current exp: %s\nNew exp: ", p->exp); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->exp,buf,sizeof(p->exp));
    printf("Current stock: %d\nNew stock: ", p->stock); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) p->stock = atoi(buf);
    printf("Current worker: %s\nNew worker name: ", p->worker_name); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->worker_name,buf,sizeof(p->worker_name));
    printf("Current worker phone: %s\nNew worker phone: ", p->worker_phone); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->worker_phone,buf,sizeof(p->worker_phone));
    printf("Current worker shift: %s\nNew worker shift: ", p->worker_shift); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) strncpy(p->worker_shift,buf,sizeof(p->worker_shift));
    printf("Current sold today: %d\nNew sold today: ", p->sold_today); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) p->sold_today = atoi(buf);
    printf("Current discount %%: %.2f\nNew discount %%: ", p->discount); if(!fgets(buf,sizeof(buf),stdin)) return; trimnl(buf); if(buf[0]) p->discount = atof(buf);
    save_db();
    printf("Edited and saved.\n");
}

/* Display total bill in a day: compute sum over sold_today * price * (1-discount/100). Also option to show per-product bill. */
void display_total_bill(void){
    double total = 0.0;
    printf("Show per-product sold & bill? (y/n): ");
    char q[8]; if(!fgets(q,sizeof(q),stdin)) return;
    int showPer = (tolower(q[0])=='y');
    if(showPer){
        printf("SNo  |  ProdID   |  Name  |   Sold  |  Discount%%   |  Bill \n");
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    }
    for(int i=0;i<product_count;i++){
        Product *p=&products[i];
        double bill = p->sold_today * p->price * (1.0 - p->discount/100.0);
        total += bill;
        if(showPer && p->sold_today>0){
            printf("%3d | %-6s | %-20s | %4d | %6.2f | %7.2f\n", p->serial,p->prod_id,p->name,p->sold_today,p->discount,bill);
        }
    }
    printf("Total daily bill (sum): %.2f\n", total);
}

/* Display workers */
void display_workers(void){
    printf("Workers details:\n");
    for(int i=0;i<5;i++){
        printf("%d) %s | Phone: %s | Shift: %s | Category: %s\n", i+1, workers[i].name, workers[i].phone, workers[i].shift, workers[i].category);
    }
}

/* Display supermarket details */
void display_supermarket(void){
    printf("Supermarket: %s\nOwner: %s | Contact: %s\nTimings: %s\n", super_name, owner_name, owner_phone, timings);
}

/* Display discounts */
void display_discounts(void){
    int found=0;
    printf("Discount offers (discount > 0):\n");
    printf("SNo | ProdID | Name | Price | Discount%%\n");
    printf("---------------------------------------\n");
    for(int i=0;i<product_count;i++){
        Product *p=&products[i];
        if(p->discount > 0.0001){
            printf("%3d | %-6s | %-20s | %7.2f | %6.2f\n",p->serial,p->prod_id,p->name,p->price,p->discount);
            found++;
        }
    }
    if(!found) printf("No discounts currently.\n");
}

/* New: export formatted record list to product_records.txt (no sold/bill columns) */
void export_to_text(void){
    FILE *fp = fopen(TEXT_EXPORT_FILE, "w");
    if(!fp){
        perror("export_to_text fopen");
        return;
    }
    fprintf(fp, "Supermarket: %s\nOwner: %s | Contact: %s\nTimings: %s\nTotal Products: %d\n\n",
            super_name, owner_name, owner_phone, timings, product_count);
    fprintf(fp, "Workers (5):\n");
    for (int w = 0; w < 5; ++w) {
        fprintf(fp, "  - %s | Phone: %s | Shift: %s | Category: %s\n",
                workers[w].name, workers[w].phone, workers[w].shift, workers[w].category);
    }
    fprintf(fp, "\n===*===========*====================================*===============*==========*============*============*=============*=======*==================*================*===========*==\n");
    fprintf(fp, "| SNo | ProdID  | Product Name                        | Category      | Price   | Mfg Date   | Exp Date   | Brand       | Stock | Worker (name)     | Worker Phone | Shift     |\n");
    fprintf(fp, "=====*===========*====================================*===============*==========*============*============*=============*=======*==================*================*===========*==\n");
    for (int i = 0; i < product_count; ++i) {
        Product *p = &products[i];
        fprintf(fp, "| %3d | %-6s  | %-34s | %-13s | %7.2f | %-10s | %-10s | %-11s | %5d | %-18s | %-13s | %-9s |\n",
                p->serial, p->prod_id, p->name, p->category, p->price, p->mfg, p->exp, p->brand,
                p->stock, p->worker_name, p->worker_phone, p->worker_shift);
    }
    fprintf(fp, "======================================================================================================================\n\n");
    // Unavailable products
    int unavailable_count = 0;
    for (int i = 0; i < product_count; ++i) if (products[i].stock == 0) unavailable_count++;
    fprintf(fp, "Unavailable products (stock = 0): %d\n", unavailable_count);
    if (unavailable_count > 0) {
        fprintf(fp, "SNo | ProdID  | Product Name                        | Category      | Brand\n");
        fprintf(fp, "-------------------------------------------------------------------------------\n");
        for (int i = 0; i < product_count; ++i) {
            if (products[i].stock == 0) {
                fprintf(fp, "%3d | %-6s  | %-34s | %-13s | %-11s\n",
                        products[i].serial, products[i].prod_id, products[i].name, products[i].category, products[i].brand);
            }
        }
        fprintf(fp, "-------------------------------------------------------------------------------\n");
    }
    fclose(fp);
    printf("Exported records to %s\n", TEXT_EXPORT_FILE);
}

/* Menu */
void menu_loop(void){
    char choice[8];
    while(1){
        printf("\n =-=-=-=-=-=-=-=-=-=-Sravya Elite Mart-=-=-=-=-==-=-=-=-=-=-\n");
        printf("<-------------------------------------------------------->\n");
        printf("||           1) Search                                  ||\n");
        printf("||           2) Add New Product                         ||\n");
        printf("||           3) Delete Product                          ||\n");
        printf("||           4) Edit Product                            ||\n");
        printf("||           5) Display All Products                    ||\n");
        printf("||           6) Display Unavailable Products            ||\n");
        printf("||           7) Display Total Bills in a Day            ||\n");
        printf("||           8) Display Workers Details                 ||\n");
        printf("||           9) Display Supermarket Details & Timings   ||\n");
        printf("||           10) Display Discount Offers                ||\n");
        printf("||           11) Save/Export Record List                ||\n");
        printf("||           12) Exit                                   ||\n");
        printf("<-------------------------------------------------------->\n");
        printf("Choose option from {1-8}: ");
        if(!fgets(choice,sizeof(choice),stdin)) break;
        int opt = atoi(choice);
        switch(opt){
            case 1: search_menu(); break;
            case 2: add_product(); break;
            case 3: delete_product(); break;
            case 4: edit_product(); break;
            case 5: display_all_products(); break;
            case 6: display_unavailable(); break;
            case 7: display_total_bill(); break;
            case 8: display_workers(); break;
            case 9: display_supermarket(); break;
            case 10: display_discounts(); break;
            case 11: export_to_text(); break;
            case 12: printf("Exiting.\n"); return;
            default: printf("Invalid option.\n"); break;
        }
        pause_continue_prompt();
    }
}

int main(void){
    if(!load_db()){
        printf("Database not found. Generating initial DB with 200 products...\n");
        generate_initial_db();
    } else {
        save_meta();
    }
    printf("Loaded %d products.\n", product_count);
    menu_loop();
    return 0;
}
// ...existing code...