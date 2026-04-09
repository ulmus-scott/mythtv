import { Component, OnInit } from '@angular/core';
import { TranslatePipe } from '@ngx-translate/core';
import { FrontendService } from '../services/frontend.service';
import { FrontendStatus } from '../interfaces/status.interface';
import { RouterLink } from "@angular/router";

@Component({
  selector: 'app-status',
  imports: [TranslatePipe, RouterLink],
  templateUrl: './status.html',
  styleUrl: './status.css',
})
export class Status implements OnInit {
    feStatus?: FrontendStatus;

    constructor(private feStatusService: FrontendService){
    }

    ngOnInit(): void {
        this.feStatusService.GetStatus().subscribe({
            next: (data) => {
                this.feStatus = data.FrontendStatus;
            },
            error: (err: any) => {
                console.log("Error getting status", err);
            }
        });

    }

}
