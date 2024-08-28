import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { ProductModel } from 'src/app/models/products.model';
import { BasketService } from 'src/app/services/basket.service';
import { environment } from 'src/environments/environment';

@Component({
  selector: 'app-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.css']
})
export class HomeComponent implements OnInit {
  api: string = environment.api;
  product: ProductModel = new ProductModel();
  products: ProductModel[] = [];
  searchText: string = '';
  showAlert: boolean = false;

  constructor(
    private _http: HttpClient,
    private _basket: BasketService
  ) { }

  ngOnInit(): void {
    this.urunListesiGetir();
  }

  urunListesiGetir() {
    this._http.get<any>(this.api + "products").subscribe({
      next: (res) => this.products = res,
      error: (err) => console.log(err)
    });
  }

  urunEkle() {
    const newCihazeUI = this.product.CihazeUI;
    const existingProduct = this.products.find(product => product.CihazeUI === newCihazeUI);

    if (existingProduct) {
      this.showAlert = true; 
      setTimeout(() => {
        this.showAlert = false; 
      }, 5000); 
    } else {
      this._http.post<any>(this.api + "products", this.product).subscribe({
        next: (res) => {
          
          this.urunListesiGetir();
          this.product = new ProductModel();
        },
        error: (err) => console.log(err)
      });
    }
  }

  CihazSil(cihazAdi: string) {
    const productToDelete = this.products.find(product => product.CihazAdi === cihazAdi);
    if (productToDelete) {
      this._http.delete<any>(`${this.api}products/${productToDelete.id}`).subscribe({
        next: () => {
          console.log("Cihaz silindi");
          this.urunListesiGetir();
        },
        error: (err) => console.log(err)
      });
    } else {
      console.log("Silinecek Cihaz bulunamadı.");
    }
  }

  CihazUrunEkle(model: ProductModel) {
    this._http.post<any>(this.api + "baskets", model).subscribe({
      next: () => {
        console.log("Cihaz eklendi");
        this.getBaskets();
      },
      error: (err) => console.log(err)
    });
  }

  getBaskets() {
    this._http.get<any>(this.api + "baskets").subscribe({
      next: (res) => this._basket.baskets = res,
      error: (err) => console.log(err)
    });
  }

 
  filterProducts(): ProductModel[] {
    return this.products.filter(product =>
      product.CihazeUI.toLowerCase().includes(this.searchText.toLowerCase())
    );
  }
}