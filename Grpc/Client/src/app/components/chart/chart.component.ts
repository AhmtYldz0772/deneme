import { Component, OnDestroy, OnInit } from '@angular/core';
import Chart from 'chart.js/auto';
import { HttpClient } from '@angular/common/http';
import { ActivatedRoute } from '@angular/router';


@Component({
  selector: 'app-chart',
  templateUrl: './chart.component.html',
  styleUrls: ['./chart.component.css']
})
export class ChartComponent implements OnInit, OnDestroy {
  chartX: any;
  chartY: any;
  chartFreqx: any;
  chartFreqy: any;
  myInterval: any;
  previousDatax = 0.0;
  previousDatay = 0.0;
  previousDataxid = 0.0;
  previousDatayid = 0.0;
  previousFreqX =  0.0;
  previousFreqY =  0.0;
  previousIdX = 0.0;
  previousIdY = 0.0;
  isFreqFirst = true;

  xArray: number[] = [];
  yArray: number[] = [];
  freqArrayx: number[] = [];
  freqArrayy: number[] = [];
  arrayLength = 1000;
  myFixedNum = 0.00000001;
  cihazEui: string = '';

  
  constructor(private http: HttpClient, private route: ActivatedRoute) {}
  ngOnInit(): void {
    this.initializeCharts();
    this.myInterval = setInterval(() => {
      this.sendRequest();
      this.updateXLabels();
      this.updateYLabels();
      this.updateFreqCharts(); 
    }, 1000);

    this.route.params.subscribe(params => {
      this.cihazEui = params['cihazEui'];
    })

}



  ngOnDestroy(): void {
    clearInterval(this.myInterval);
    if (this.chartX) {
      this.chartX.destroy();
    }
    if (this.chartY) {
      this.chartY.destroy();
    }
    if (this.chartFreqx) {
      this.chartFreqx.destroy();
    }
    if (this.chartFreqy) {
      this.chartFreqy.destroy();
    }
  }

  initializeCharts(): void {
    const labels = Array.from({ length: this.arrayLength }, (_, i) => {
      const now = new Date();
      const startTime = new Date(now.getTime() - this.arrayLength * 1000);
      const time = new Date(startTime.getTime() + i * 1000);
      return `${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`;
    });

    this.chartX = new Chart('chartX', {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Sensör Verisi X',
          data: this.xArray,
          borderColor: 'blue',
          fill: false,
        }]
      },
      options: {
        scales: {
          x: {
            type: 'category',
            offset: true,
          }
        },
        animation: {
          duration: 1000,
          easing: 'linear',
        },
      }
    });

    this.chartY = new Chart('chartY', {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Sensör Verisi Y',
          data: this.yArray,
          borderColor: 'red',
          fill: false,
        }]
      },
      options: {
        scales: {
          x: {
            type: 'category',
            offset: true,
          }
        },
        animation: {
          duration: 1000,
          easing: 'linear',
        },
      }
    });

    this.chartFreqx = new Chart('chartFreqx', {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'FreqX Verisi',
          data: this.freqArrayx,
          borderColor: 'green',
          fill: false,
        }]
      },
      options: {
        scales: {
          x: {
            type: 'category',
            offset: true,
          }
        },
        animation: {
          duration: 1000,
          easing: 'linear',
        },
      }
    });

    this.chartFreqy = new Chart('chartFreqy', {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'FreqY Verisi',
          data: this.freqArrayy,
          borderColor: 'green',
          fill: false,
        }]
      },
      options: {
        scales: {
          x: {
            type: 'category',
            offset: true,
          }
        },
        animation: {
          duration: 1000,
          easing: 'linear',
        },
      }
    });

    //this.sendRequest();
  }

 updateChartWithData(data: any): void {
  if (!data || data.length === 0) {
    console.error('Gelen veride eksiklik var veya boş:', data);
    return;
  }


  const latestDatax = data.find((item: any) => item.typeeksen == 171);
  
  const latestDatay = data.find((item: any) => item.typeeksen == 205);

  if (!latestDatax || !latestDatay) {
    console.error('X veya Y verisi bulunamadı:', data);
    return;
  }

  const latestidX = parseInt(latestDatax.id || 0);
  const latestidY = parseInt(latestDatay.id || 0);

  if (this.previousDataxid == latestidX) {
    this.updateChartWithFixedData(0);
  } else {
    const x_value = latestDatax.typeeksen == 171 ? latestDatax.peak : this.myFixedNum;
    this.xArray.splice(0, 1);
    this.xArray[this.arrayLength - 1] = x_value;
    this.chartX.data.datasets[0].data = this.xArray;
    this.chartX.update('none');
    this.previousDataxid = latestidX;
  }

  if (this.previousDatayid == latestidY) {
    this.updateChartWithFixedData(1);
  } else {
    const y_value = latestDatay.typeeksen == 205 ? latestDatay.peak : this.myFixedNum;
    this.yArray.splice(0, 1);
    this.yArray[this.arrayLength - 1] = y_value;
    this.chartY.data.datasets[0].data = this.yArray;
    this.chartY.update('none');
    this.previousDatayid = latestidY;
  }
}
  updateChartWithFixedData(type:any): void {

    //this.shiftArray();
    if(type === 0){     
      this.xArray.splice(0,1);
    }

    if(type === 1){
      this.yArray.splice(0,1);
    }
  
    if(type === 0)
      this.xArray[this.arrayLength - 1] = this.myFixedNum;
    if(type === 1)
      this.yArray[this.arrayLength - 1] = this.myFixedNum;

    /*
    for (let i = this.arrayLength ; i < this.arrayLength; i++) {
      this.xArray[i] = this.myFixedNum;
      this.yArray[i] = this.myFixedNum;
    }*/

    if(type === 0)
      this.chartX.data.datasets[0].data = this.xArray;
    if(type === 1)
      this.chartY.data.datasets[0].data = this.yArray;

    if(type === 0)
      this.chartX.update('none');
    if(type === 1)
      this.chartY.update('none');
  }

  sendRequest(): void {
    this.http.get(`http://localhost:5000/get_device_data?cihaz_eui=${this.cihazEui}`, { responseType: 'json' }).subscribe(
      (data: any) => {
        //console.log(data)
        if (this.previousDataxid === 0.0 && this.previousDatayid === 0.0) {
          const firstDatax = data.find((item: any) => item.typeeksen == 171) || { peak: 0.0 };
          const firstDatay = data.find((item: any) => item.typeeksen == 205) || { peak: 0.0 };
          if (firstDatax && firstDatay) {
            this.previousDataxid = firstDatax.id || 0;
            this.previousDatayid = firstDatay.id || 0;
          }
        }
        this.updateChartWithData(data);
        this.updateXLabels();
        this.updateYLabels();
      },
      (error) => {
        console.error('Veri alınamadı:', error);
      }
    );
  }

  updateFreqCharts(): void {
    this.http.get<any[]>(`http://localhost:5000/tabloFreq?cihaz_eui=${this.cihazEui}`, { responseType: "json" }).subscribe(
      (data) => {
        
        this.updateFREQXLabels();
        this.updateFREQYLabels();

      
  
        if (this.previousIdX === 0) {
          const firstDataX = data.find((item) => item.typeeksen == 171); 
          if (firstDataX) {
            this.previousIdX = parseInt(firstDataX.id || 0);
          }
        }

        if (this.previousIdY === 0) {
          const firstDataY = data.find((item) => item.typeeksen == 205);
          if (firstDataY) {
            this.previousIdY = parseInt(firstDataY.id || 0);
          }
        }
        
        
        if (data && data.length > 0) {  
          const latestDataX = data.find((item) => item.typeeksen == 171);
          const latestDataY = data.find((item) => item.typeeksen == 205);
          
          if (latestDataX) {
            const latestIdX = latestDataX.id || 0;
  
            this.freqArrayx.splice(0, 1);
  
            if (this.previousIdX !== latestIdX) {
              const latestFreqX = parseFloat(latestDataX.freq || 0);
              this.freqArrayx[this.arrayLength - 1] = latestFreqX;
              this.chartFreqx.data.datasets[0].data = this.freqArrayx;
              this.chartFreqx.update();
              this.previousIdX = latestIdX;
              
            } else {
              this.freqArrayx[this.arrayLength - 1] = 0;
              this.chartFreqx.update();
            }
          }

          if (latestDataY) {
            const latestIdY = latestDataY.id || 0;

            this.freqArrayy.splice(0, 1);

            if (this.previousIdY !== latestIdY) {
              const latestFreqY = parseFloat(latestDataY.freq || 0);
              this.freqArrayy[this.arrayLength - 1] = latestFreqY;
              this.chartFreqy.data.datasets[0].data = this.freqArrayy;
              this.chartFreqy.update();
              this.previousIdY = latestIdY;
            } else {
              this.freqArrayy[this.arrayLength - 1] = 0;
              this.chartFreqy.update();
            }

          }
        }
      },
      (error) => {
        console.error('Veri alınamadı:', error);
      }
    );
  }
  

  updateFREQXLabels(): void {
    const now = new Date();
    const startTime = new Date(now.getTime() - this.arrayLength * 1000);

    const labels = Array.from({ length: this.arrayLength }, (_, i) => {
      const time = new Date(startTime.getTime() + i * 1000);
      return `${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`;
    });

    this.chartFreqx.data.labels = labels;
    this.chartFreqx.update();
  }


  updateFREQYLabels(): void {
    const now = new Date();
    const startTime = new Date(now.getTime() - this.arrayLength * 1000);

    const labels = Array.from({ length: this.arrayLength }, (_, i) => {
      const time = new Date(startTime.getTime() + i * 1000);
      return `${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`;
    });

    this.chartFreqy.data.labels = labels;
    this.chartFreqy.update();
  }


  updateXLabels(): void {
    const now = new Date();
    const startTime = new Date(now.getTime() - this.arrayLength * 1000);

    const labels = Array.from({ length: this.arrayLength }, (_, i) => {
      const time = new Date(startTime.getTime() + i * 1000);
      return `${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`;
    });

    this.chartX.data.labels = labels;
    this.chartX.update();
  }

  updateYLabels(): void {
    const now = new Date();
    const startTime = new Date(now.getTime() - this.arrayLength * 1000);

    const labels = Array.from({ length: this.arrayLength }, (_, i) => {
      const time = new Date(startTime.getTime() + i * 1000);
      return `${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`;
    });

    this.chartY.data.labels = labels;
    this.chartY.update();
  }

  shiftArray(): void {
    for (let i = 0; i < this.arrayLength; i++) {
      if (i >= 5) {
        this.xArray[i - 10] = this.xArray[i];
        this.yArray[i - 10] = this.yArray[i];
        
      }
      if (i >= 5){
        this.freqArrayx[i-10] = this.freqArrayx[i];
        this.freqArrayy[i-10] = this.freqArrayy[i];
      }
    }
  }

  shift_and_delete(type :any): void {

    if(type === 0){     
        this.xArray.splice(0,1);
        this.freqArrayx.splice(0,1);
    }

    if(type === 1){
        this.yArray.splice(0,1);
        this.freqArrayy.splice(0,1);
    }

   }
  



 /* onCihazEuiInputChange(event: any): void {
    this.cihazEui = event.target.value;
  }

  onButtonClick(): void {
    this.sendRequest();
  }*/

}
