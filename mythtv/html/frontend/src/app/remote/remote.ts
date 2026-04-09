import { Component } from '@angular/core';
import { RouterLink } from "@angular/router";
import { TranslatePipe } from '@ngx-translate/core';
import { FrontendService } from '../services/frontend.service';

@Component({
  selector: 'app-remote',
  imports: [TranslatePipe, RouterLink],
  templateUrl: './remote.html',
  styleUrl: './remote.css',
})
export class Remote {

    constructor(private frontendService: FrontendService) {
    }

    action(bnCode: string) {
        console.log(bnCode)
        this.frontendService.SendAction(bnCode).subscribe(this.observer);
    }

    observer = {
        next: (x: any) => {
            if (!x.bool) {
                console.log("Invalid response to action:" + x);
            }
        },
        error: (err: any) => {
            console.error(err);
        },
    };

}
