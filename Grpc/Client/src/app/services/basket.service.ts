import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { environment } from 'src/environments/environment';
import { BasketModel } from '../models/basket.model';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class BasketService {
 

  api: string = environment.api;
  api1 : string = environment.api1;
  baskets: BasketModel[] = [];

  constructor(
    private _http: HttpClient
  ) { 
    this.getBaskets();
  }

  delete(id: number){
    this._http.delete<any>(this.api + "baskets/" + id).subscribe({
      next: ()=> this.getBaskets(),
      error: (err)=> console.log(err)      
    })
  }

  sendData(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/send_data`, {CihazeUI});
  }
  device_info_set(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/send_status`, {CihazeUI});
  }
  sendEvent(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/sendEvent`, {CihazeUI});
  }
  kalibrasyon(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/sendkalibrasyon`, {CihazeUI});
  }
  reset(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/send_reset`, {CihazeUI});
  }
  tabloEvent(CihazeUI:string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/tablo`, {CihazeUI});
  }
  
  setThreshold(value: number, CihazeUI: string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/Set_Threshold`, { value, CihazeUI});
  }
  getThresholdValue(CihazeUI: string) {
    return this._http.get<any>(`${environment.api1}/Get_Threshold`, { params: { CihazeUI } });
  }


  setFrequency(value: number, id:number, CihazeUI: string): Observable<any> {
    return this._http.post<any>(`${environment.api1}/Set_fREAQUESNCY`, { value,  CihazeUI });
  }

  getBaskets(){
    this._http.get<any>(this.api + "baskets").subscribe({
      next: (res)=> this.baskets = res,
      error: (err)=> console.log(err)      
    })
  }
  update(id: number, updatedBasket: BasketModel) {
    this._http.put<any>(this.api + "baskets/" + id, updatedBasket).subscribe({
      next: ()=> this.getBaskets(),
      error: (err)=> console.log(err)      
    })
  }
}