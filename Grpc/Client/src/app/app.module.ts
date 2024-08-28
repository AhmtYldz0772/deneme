import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpClientModule  } from "@angular/common/http";

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { LayoutsComponent } from './components/layouts/layouts.component';
import { HomeComponent } from './components/home/home.component';
import { FormsModule } from '@angular/forms';
import { BasketComponent } from './components/basket/basket.component';
import { ChartComponent } from './components/chart/chart.component';
import { DynamicChartComponent } from './dynamic-chart/dynamic-chart.component';







@NgModule({
  declarations: [
    AppComponent,
    LayoutsComponent,
    HomeComponent,
    BasketComponent,
    ChartComponent,
    DynamicChartComponent,


    
  ],
  imports: [
    BrowserModule,
    FormsModule,
    HttpClientModule,
    AppRoutingModule

  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
