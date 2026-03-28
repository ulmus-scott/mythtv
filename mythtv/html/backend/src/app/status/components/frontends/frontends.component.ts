import { Component, OnInit, Input } from '@angular/core';
import { Frontend } from "src/app/services/interfaces/frontend.interface";
import { TranslateModule } from '@ngx-translate/core';


@Component({
    selector: 'app-status-frontends',
    templateUrl: './frontends.component.html',
    styleUrls: ['./frontends.component.css', '../../status.component.css'],
    imports: [TranslateModule]
})
export class FrontendsComponent implements OnInit {
    @Input() frontends?: Frontend[];

    constructor() { }

    ngOnInit(): void {
    }

}
