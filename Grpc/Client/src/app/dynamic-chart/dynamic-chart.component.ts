import { Component, OnDestroy, OnInit } from '@angular/core';
import Chart from 'chart.js/auto';
import { HttpClient } from '@angular/common/http';
import { ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-dynamic-chart',
  templateUrl: './dynamic-chart.component.html',
  styleUrls: ['./dynamic-chart.component.css']
})
export class DynamicChartComponent implements OnInit, OnDestroy {
  chartPeakX: any;
  chartPeakY: any;
  chartThresholdX: any;
  chartThresholdY: any;
  myInterval: any;
  previousPeakDataIdX = 0;
  previousPeakDataIdY = 0;
  previousThresholdDataIdX = 0;
  previousThresholdDataIdY = 0;
  previousIdX = 0;
  previousIdY = 0;
  peakDataArrayX: number[] = Array(100).fill(0);
  peakDataArrayY: number[] = Array(100).fill(0);
  thresholdDataArrayX: number[] = Array(100).fill(0);
  thresholdDataArrayY: number[] = Array(100).fill(0);
  arrayLength = 100;
  cihazEui: string = '';
  labels: string[] = Array.from({ length: 100 }, (_, i) => new Date(Date.now() - (100 - i) * 1000).toLocaleTimeString());

  constructor(private http: HttpClient, private route: ActivatedRoute) {}

  ngOnInit(): void {
    this.route.params.subscribe(params => {
      this.cihazEui = params['cihazEui'];
      this.initializeCharts();
      this.myInterval = setInterval(() => {
        this.sendRequest();
        this.updateFreqCharts();
      }, 1000);
    });
  }

  ngOnDestroy(): void {
    clearInterval(this.myInterval);
    if (this.chartPeakX) {
      this.chartPeakX.destroy();
    }
    if (this.chartPeakY) {
      this.chartPeakY.destroy();
    }
    if (this.chartThresholdX) {
      this.chartThresholdX.destroy();
    }
    if (this.chartThresholdY) {
      this.chartThresholdY.destroy();
    }
  }
  

  initializeCharts(): void {
    this.chartPeakX = new Chart('chartPeakX', {
      type: 'line',
      data: {
        labels: this.labels,
        datasets: [{
          label: 'Threshold Verisi X',
          data: this.peakDataArrayX,
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

    this.chartPeakY = new Chart('chartPeakY', {
      type: 'line',
      data: {
        labels: this.labels,
        datasets: [{
          label: 'Threshold Verisi Y',
          data: this.peakDataArrayY,
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

    this.chartThresholdX = new Chart('chartThresholdX', {
      type: 'line',
      data: {
        labels: this.labels,
        datasets: [{
          label: 'Peak Verisi X',
          data: this.thresholdDataArrayX,
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

    this.chartThresholdY = new Chart('chartThresholdY', {
      type: 'line',
      data: {
        labels: this.labels,
        datasets: [{
          label: 'Peak Verisi Y',
          data: this.thresholdDataArrayY,
          borderColor: 'purple',
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
  }

  updateChartsWithData(data: any): void {
    if (!data || data.length === 0) {
      console.error('Gelen veride eksiklik var veya boş:', data);
      return;
    }

    const latestPeakDataX = data.find((item: any) => item.typeeksen == 171);
    const latestPeakDataY = data.find((item: any) => item.typeeksen == 205);

    const currentTime = new Date().toLocaleTimeString();

    if (latestPeakDataX && this.previousPeakDataIdX !== latestPeakDataX.id) {
      this.labels.push(currentTime);
      if (this.labels.length > this.arrayLength) {
        this.labels.shift();
      }

      this.peakDataArrayX.splice(0, 1);
      this.peakDataArrayX.push(latestPeakDataX.peak || 0);
      this.chartPeakX.data.datasets[0].data = this.peakDataArrayX;
      this.chartPeakX.data.labels = this.labels;
      this.chartPeakX.update('none');
      this.previousPeakDataIdX = latestPeakDataX.id;
    } else if (!latestPeakDataX) {
      this.labels.push(currentTime);
      if (this.labels.length > this.arrayLength) {
        this.labels.shift();
      }

      this.peakDataArrayX.splice(0, 1);
      this.peakDataArrayX.push(0);
      this.chartPeakX.data.datasets[0].data = this.peakDataArrayX;
      this.chartPeakX.data.labels = this.labels;
      this.chartPeakX.update('none');
    }

    if (latestPeakDataY && this.previousPeakDataIdY !== latestPeakDataY.id) {
      this.peakDataArrayY.splice(0, 1);
      this.peakDataArrayY.push(latestPeakDataY.peak || 0);
      this.chartPeakY.data.datasets[0].data = this.peakDataArrayY;
      this.chartPeakY.update('none');
      this.previousPeakDataIdY = latestPeakDataY.id;
    } else if (!latestPeakDataY) {
      this.peakDataArrayY.splice(0, 1);
      this.peakDataArrayY.push(0);
      this.chartPeakY.data.datasets[0].data = this.peakDataArrayY;
      this.chartPeakY.update('none');
    }
  }

  updateFreqCharts(): void {
    this.http.get<any[]>(`http://localhost:5000/tabloFreq?cihaz_eui=${this.cihazEui}`, { responseType: "json" }).subscribe(
      (data) => {
        if (!data || data.length === 0) {
          console.error('Gelen veride eksiklik var veya boş:', data);
          return;
        }

        const latestThresholdDataX = data.find((item) => item.typeeksen == 171);
        const latestThresholdDataY = data.find((item) => item.typeeksen == 205);

        const currentTime = new Date().toLocaleTimeString();

        if (latestThresholdDataX && this.previousThresholdDataIdX !== latestThresholdDataX.id) {
          this.labels.push(currentTime);
          if (this.labels.length > this.arrayLength) {
            this.labels.shift();
          }

          this.thresholdDataArrayX.splice(0, 1);
          this.thresholdDataArrayX.push(latestThresholdDataX.freq || 0);
          this.chartThresholdX.data.datasets[0].data = this.thresholdDataArrayX;
          this.chartThresholdX.data.labels = this.labels;
          this.chartThresholdX.update('none');
          this.previousThresholdDataIdX = latestThresholdDataX.id;
        } else if (!latestThresholdDataX) {
          this.labels.push(currentTime);
          if (this.labels.length > this.arrayLength) {
            this.labels.shift();
          }

          this.thresholdDataArrayX.splice(0, 1);
          this.thresholdDataArrayX.push(0);
          this.chartThresholdX.data.datasets[0].data = this.thresholdDataArrayX;
          this.chartThresholdX.data.labels = this.labels;
          this.chartThresholdX.update('none');
        }

        if (latestThresholdDataY && this.previousThresholdDataIdY !== latestThresholdDataY.id) {
          this.thresholdDataArrayY.splice(0, 1);
          this.thresholdDataArrayY.push(latestThresholdDataY.freq || 0);
          this.chartThresholdY.data.datasets[0].data = this.thresholdDataArrayY;
          this.chartThresholdY.update('none');
          this.previousThresholdDataIdY = latestThresholdDataY.id;
        } else if (!latestThresholdDataY) {
          this.thresholdDataArrayY.splice(0, 1);
          this.thresholdDataArrayY.push(0);
          this.chartThresholdY.data.datasets[0].data = this.thresholdDataArrayY;
          this.chartThresholdY.update('none');
        }
      },
      (error) => {
        console.error('Veri alınamadı:', error);
      }
    );
  }

  sendRequest(): void {
    this.http.get(`http://localhost:5000/get_device_data?cihaz_eui=${this.cihazEui}`, { responseType: 'json' }).subscribe(
      (data: any) => {
        this.updateChartsWithData(data);
      },
      (error) => {
        console.error('Veri alınamadı:', error);
      }
    );
  }
}
