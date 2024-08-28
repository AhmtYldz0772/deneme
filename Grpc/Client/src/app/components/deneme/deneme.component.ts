import { HttpClient } from '@angular/common/http';
import { AfterViewInit, Component } from '@angular/core';
import { CanvasJSAngularChartsModule } from '@canvasjs/angular-charts';

declare var CanvasJS: any;

@Component({
  selector: 'app-deneme',
  templateUrl: './deneme.component.html',
  styleUrls: ['./deneme.component.css']
})
export class DenemeComponent implements AfterViewInit {
  dataPoints1: any[] = [];
  dataPoints2: any[] = [];
  chart: any;
  updateInterval = 2000;
  yValue1 = 90;
  yValue2 = 97;
  time = new Date();
  cihazEui = 'cihazEuiDeğeriniz'; // gerekli cihaz EUI değerini buraya girin

  chartOptions = {
    zoomEnabled: true,
    theme: "dark2",
    title: {
      text: "Sensör Verileri"
    },
    axisX: {
      title: "Zaman"
    },
    axisY: {
      prefix: "$"
    },
    toolTip: {
      shared: true
    },
    legend: {
      cursor: "pointer",
      verticalAlign: "top",
      fontSize: 22,
      fontColor: "dimGrey",
      itemclick: function (e: any) {
        if (typeof (e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
          e.dataSeries.visible = false;
        } else {
          e.dataSeries.visible = true;
        }
        e.chart.render();
      }
    },
    data: [{ 
      type: "line",
      xValueType: "dateTime",
      yValueFormatString: "$####.00",
      xValueFormatString: "hh:mm:ss TT",
      showInLegend: true,
      name: "Company A",
      dataPoints: this.dataPoints1
    }, {				
      type: "line",
      xValueType: "dateTime",
      yValueFormatString: "$####.00",
      showInLegend: true,
      name: "Company B",
      dataPoints: this.dataPoints2
    }]
  };

  constructor(private httpClient: HttpClient) {}

  ngAfterViewInit() {
    this.setupChart();
    setInterval(() => {
      this.updateChart(1); 
    }, this.updateInterval);	
  }

  private setupChart(): void {
    // Initialize chart setup here
    this.chart = new CanvasJS.Chart("chartContainer", this.chartOptions);
    this.chart.render();
  }

  private updateChart(count: number): void {
    let deltaY1, deltaY2;
    for (let i = 0; i < count; i++) {
      this.time.setTime(this.time.getTime() + this.updateInterval);
      deltaY1 = .5 + Math.random() *(-.5-.5);
      deltaY2 = .5 + Math.random() *(-.5-.5);

      this.yValue1 = Math.round((this.yValue1 + deltaY1) * 100) / 100;
      this.yValue2 = Math.round((this.yValue2 + deltaY2) * 100) / 100;

      this.dataPoints1.push({
        x: this.time.getTime(),
        y: this.yValue1
      });
      this.dataPoints2.push({
        x: this.time.getTime(),
        y: this.yValue2
      });
    }

    this.chart.options.data[0].legendText = " Company A  $" + CanvasJS.formatNumber(this.yValue1, "#,###.00");
    this.chart.options.data[1].legendText = " Company B  $" + CanvasJS.formatNumber(this.yValue2, "#,###.00"); 
    this.chart.render();
  }
}
