import { Component, OnInit } from '@angular/core';
import { DataService } from 'src/app/services/data.service';
import { TranslatePipe } from '@ngx-translate/core';
import { RippleModule } from 'primeng/ripple';
import { TooltipModule } from 'primeng/tooltip';
import { RouterLink, RouterOutlet } from '@angular/router';

@Component({
    selector: 'app-sidenav',
    templateUrl: './sidenav.component.html',
    styleUrls: ['./sidenav.component.css'],
    imports: [RouterLink, TooltipModule, RippleModule, RouterOutlet, TranslatePipe]
})

export class SidenavComponent implements OnInit {

    constructor(public dataService: DataService) { }

    ngOnInit(): void {
    }
}
