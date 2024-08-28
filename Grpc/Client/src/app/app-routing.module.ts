import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { BasketComponent } from './components/basket/basket.component';
import { HomeComponent } from './components/home/home.component';
import { LayoutsComponent } from './components/layouts/layouts.component';
import { ChartComponent } from './components/chart/chart.component';
import { DynamicChartComponent } from './dynamic-chart/dynamic-chart.component';

const routes: Routes = [
  {
    path: "",
    component: LayoutsComponent,
    children: [
      {
        path: "",
        component: HomeComponent
      },
      {
        path: "basket",
        component: BasketComponent
      },
      {
        path: "chart",
        component: ChartComponent
      },
      {
        path: "dynamic-chart",
        component: DynamicChartComponent
      },
      
    ]
  }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
