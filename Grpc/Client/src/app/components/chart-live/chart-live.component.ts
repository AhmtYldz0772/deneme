
/* app.component.ts */
import { Component, AfterViewInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RouterOutlet } from '@angular/router';
declare var require: any;
if(typeof document === 'object' && !!document) {
  var CanvasJS = require('../../node_modules/@canvasjs/charts');
}
 

 
@Component({
  selector: 'app-root',
  standalone: true,
  
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
 
export class AppComponent implements AfterViewInit{
  dataPoints1: any[] = [];
  dataPoints2: any[] = [];
 
  chart: any;
 
  chartOptions = {
    zoomEnabled: true,
    theme: "dark2",
    title: {
      text: "Share Value of Two Companies"
    },
    axisX: {
      title: "chart updates every 2 secs"
    },
    axisY:{
      prefix: "$"
    }, 
    toolTip: {
      shared: true
    },
    legend: {
      cursor:"pointer",
      verticalAlign: "top",
      fontSize: 22,
      fontColor: "dimGrey",
      itemclick : function(e:any) {
        if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
          e.dataSeries.visible = false;
        }
        else {
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
      name: "Company B" ,
      dataPoints: this.dataPoints2
    }]
  }
 
  getChartInstance(chart: object) {
    this.chart = chart;
    
    this.time.setHours(9);
    this.time.setMinutes(30);
    this.time.setSeconds(0);
    this.time.setMilliseconds(0);
    this.updateChart(100);
  }
 
  updateInterval = 2000;
  
  // initial value
  yValue1 = 90; 
  yValue2 = 97;
  time = new Date();
 
  updateChart = (count: any) => {
    count = count || 1;
    var deltaY1, deltaY2;
    for (var i = 0; i < count; i++) {
      this.time.setTime(this.time.getTime()+ this.updateInterval);
      deltaY1 = .5 + Math.random() *(-.5-.5);
      deltaY2 = .5 + Math.random() *(-.5-.5);
    
      // adding random value and rounding it to two digits. 
      this.yValue1 = Math.round((this.yValue1 + deltaY1)*100)/100;
      this.yValue2 = Math.round((this.yValue2 + deltaY2)*100)/100;
    
      // pushing the new values
      this.dataPoints1.push({
        x: this.time.getTime(),
        y: this.yValue1
      });
      this.dataPoints2.push({
        x: this.time.getTime(),
        y: this.yValue2
      });
    }
  
    // updating legend text with  updated with y Value 
    this.chart.options.data[0].legendText = " Company A  $" + CanvasJS.formatNumber(this.yValue1, "#,###.00");
    this.chart.options.data[1].legendText = " Company B  $" + CanvasJS.formatNumber(this.yValue2, "#,###.00"); 
    this.chart.render();
  }
  ngAfterViewInit() {
    setInterval(() => {
      this.updateChart(1); 
    }, this.updateInterval);	
  } 
} 