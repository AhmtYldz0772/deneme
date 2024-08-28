import { AfterContentChecked, Component, Input, OnInit,ViewChild } from '@angular/core';
import { BasketModel } from 'src/app/models/basket.model';
import { BasketService } from 'src/app/services/basket.service';
import { HttpClient } from '@angular/common/http';


@Component({
  selector: 'app-basket',
  templateUrl: './basket.component.html',
  styleUrls: ['./basket.component.css']
})
export class BasketComponent implements AfterContentChecked, OnInit {
  baskets: BasketModel[] = [];
  newEventData: boolean = false; 
  eventData: string[] = []; 
  lastEventData: string[] = []; 
  searchText: string = ''; 
  tableData: any[] = [];
  pageSize: number = 20;
  currentPage: number = 1;
  totalPages: number = 0;
  displayedTableData: any[] = [];
  thresholdValuex: number = 0.001;
  thresholdValuey: number = 0.001;
  newThresholdValue: number = 0.00001;
  oldThresholdValue: number = 0.00000;
  selectedBasketCihazeUI: string = '';
  inputValuesCihazEUI: string = "";
  inputValuesTip: string = "";
  inputValuesAltip: string = "";
  inputValuesTime: string = "";
  inputValuesType: string = "";
  inputValuesPeak2: any = "";
  inputValuesFreq: any = "";
  x_value = 0;
  y_value = 0;
  sensor_time: string[] = [];
  constructor(
    private _basket: BasketService,
    private _http: HttpClient
  ) { 
    setInterval(() => {
      this.getEventData();
      this.sendInputValuesToServer();
      this.device_info_get();
      
    }, 1000);
  }


  sendInputValuesToServer() {
  const inputValues = {
    cihaz_eui: this.inputValuesCihazEUI,
    tip: this.inputValuesTip,
    altip: this.inputValuesAltip,
    zaman: this.inputValuesTime,
    type: this.inputValuesType,
    peak2: this.inputValuesPeak2,
    freq: this.inputValuesFreq
  };
  

  if (Object.values(inputValues).some(val => typeof val === 'string' && val.trim() !== '')) {
    this._http.post<any>('http://localhost:5000/tablo_filitre', inputValues).subscribe(
      (data) => {
        console.log(data);
        this.tableData = data;
        this.calculateTotalPages();
        this.updateDisplayedTableData();
        this.filterTable()
      },
      (error) => {
        console.error('Tablo verilerini alma hatası:', error);
        console.error('Hata detayları:', error.message);
      }
    );
  } else {
    
    this.fetchTableData();
  }
}
confirmAndSendEvent(CihazeUI: string) {
  const confirmed = confirm("Olayları göndermek istediğinize emin misiniz?");
  if (confirmed) {
    this.sendEvent(CihazeUI);
    this.tabloEvent(CihazeUI);
  } else {
    console.log("İşlem iptal edildi.");
  }
}

confirmanThreshold(CihazeUI: string){
  const confirmed = confirm("threshold değerini güncelemek istediğine emin misisn?")
  if(confirmed){
    this.getThreshold(CihazeUI)
  }
  else{
    console.log("işlem iptal edildi")
  }
}

filterTable() {
  const searchText = this.searchText.toLowerCase();
  this.displayedTableData = this.tableData.filter(item => {
    return item.zaman && item.zaman.toLowerCase().includes(searchText);
  });
}
  ngOnInit(): void {
    this.fetchTableData();
  }

  ngAfterContentChecked(): void {
    this.baskets = this._basket.baskets;
    this.calculateTotalPages(); 
    this.updateDisplayedTableData(); 
  
  }  

  filterBaskets(): BasketModel[] {
    return this.baskets.filter(basket =>
      basket.CihazeUI.toLowerCase().includes(this.searchText.toLowerCase())
    );
  } 
 


  kalibrasyon(CihazeUI:string) {
    if (confirm(`Cihazı ${CihazeUI} için kalibre etmek istediğinize emin misiniz?`)) {
      this._basket.kalibrasyon(CihazeUI).subscribe(
        () => {
          console.log(`${CihazeUI} için cihaz kalibrasyonu tamamlandı`);
        },
        (error) => {
          console.error('Cihaz olayı gönderme hatası:', error);
        }
      );
  }
  }

  reset(CihazeUI:string) {
    if (confirm(`Cihazı ${CihazeUI} için resetlemek istediğinize emin misiniz?`)) {
        this._basket.reset(CihazeUI).subscribe(
          () => {
            console.log(`${CihazeUI} için cihaz reset tamamlandı`);
          },
          (error) => {
            console.error('Cihaz olayı gönderme hatası:', error);
          }
        );
        }else {
          console.log('Reset işlemi iptal edildi.');
      }
  }

// tablo olayları bağlantı ve veri tabanından veri çekme
  tabloEvent(CihazeUI:string){
    this._basket.tabloEvent(CihazeUI).subscribe(
      () => {
        console.log(`${CihazeUI} için cihaz olayı gönderildi`);
      },
      (error) => {
        console.error('Cihaz olayı gönderme hatası:', error);
      }
    );
  }

  fetchTableData() {
    this._http.get<any[]>('http://localhost:5000/tabloEvent').subscribe(
      (data) => {
        this.tableData = this.makeDataPositive(data);
        this.calculateTotalPages();
        this.updateDisplayedTableData();
        this.filterTable()
      },
      (error) => {
        console.error('Tablo verilerini alma hatası:', error);
        console.error('Hata detayları:', error.message);
      }
    );
  }
// kısmının bağlantı ve veri tabaınından verilerin çekilmesi
  getEventData() {
    this._http.get('http://localhost:5000/event', { responseType: 'text' }).subscribe(
      (data) => {
        try {
          const lines = data.split('\n'); 
          const lastLines = lines.slice(-100); 
          if (lastLines.join('\n') !== this.lastEventData.join('\n')) {
            this.newEventData = true;
            this.lastEventData = [...this.eventData]; 
          }
          this.eventData = lastLines; 
        } catch (error) {
          console.error('JSON ayrıştırma hatası:', error);
        }
      },
      (error) => {
        console.error('Olay verisi alınırken hata:', error);
      }
    );
  }

  onEventsClick() {
    this.newEventData = false;
    this.getEventData();
  }

  sendEvent(CihazeUI:string) {
    this._basket.sendEvent(CihazeUI).subscribe(
      () => {
        console.log(`${CihazeUI} için cihaz olayı gönderildi`);
      },
      (error) => {
        console.error('Cihaz olayı gönderme hatası:', error);
      }
    );
  }
  
  makeDataPositive(data: any[]): any[] {
    for (let i = 0; i < data.length; i++) {
      for (const key in data[i]) {
        if (typeof data[i][key] === 'number' && data[i][key] < 0) {
          data[i][key] = Math.abs(data[i][key]);
        }
      }
    }
    return data;
  }

  calculateTotalPages() {
    this.totalPages = Math.ceil(this.tableData.length / this.pageSize);
  }

  updateDisplayedTableData() {
    const startIndex = (this.currentPage - 1) * this.pageSize;
    const endIndex = Math.min(startIndex + this.pageSize, this.tableData.length);
    this.displayedTableData = this.tableData.slice(startIndex, endIndex);
  }
  

  previousPage() {
    if (this.currentPage > 1) {
      this.currentPage--;
      this.updateDisplayedTableData();
    }
  }

  nextPage() {
    if (this.currentPage < this.totalPages) {
      this.currentPage++;
      this.updateDisplayedTableData();
    }
  }

 

  delete(id: number){
    this._basket.delete(id);
  }


  deviceData: any[] = [];
  device_info_get() {
      this._http.get<any[]>('http://localhost:5000/device_status').subscribe(
          (data) => {
              this.deviceData = data;
              //console.log('Cihaz bilgileri:', data);
          },
          (error) => {
              console.error('Cihaz bilgisi alma hatası:', error);
              console.error('Hata detayları:', error.message);
          }
      );
  }

  device_info(CihazeUI:string){
    this._basket.device_info_set(CihazeUI).subscribe(
      () => {
        console.log('info gönderildi');
      },
      (error) => {
        console.error('info gönderme hatası:', error);
      }
    );
  }
  sendDataToServer(CihazeUI:string) {
    this._basket.sendData(CihazeUI).subscribe(
      () => {
        console.log('Zaman güncelleme isteği gönderildi');
      },
      (error) => {
        console.error('Zaman güncelleme isteği gönderme hatası:', error);
      }
    );
  }


 


  getThreshold(CihazeUI: string) {
    this._basket.getThresholdValue(CihazeUI).subscribe(
      (data) => {
        this.oldThresholdValue = this.thresholdValuex;
        this.thresholdValuex = data[0];
        this.thresholdValuey = data[1];
        this.selectedBasketCihazeUI = CihazeUI;
        console.log(data); 
      },
      (error) => {
        console.error('Threshold değeri alınamadı:', error);
      }
    );
  }

 onThresholdClick(newThresholdValue: number ) {
    const CihazeUI = this.selectedBasketCihazeUI; 
    if (!CihazeUI) {
        console.error('CihazeUI değeri boş veya tanımsız.');
        return;
    }
    this._basket.setThreshold(newThresholdValue, CihazeUI).subscribe(
        (data) => {       
            console.log('Threshold gönderildi:', data);
            this.thresholdValuex = newThresholdValue;
            this.getThreshold(CihazeUI);
        },
        
    );
}
}