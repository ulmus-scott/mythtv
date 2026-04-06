import { Component, OnInit, Input } from '@angular/core';
import { Frontend } from "../../../services/interfaces/frontend.interface";
import { TranslatePipe } from '@ngx-translate/core';


@Component({
    selector: 'app-status-frontends',
    templateUrl: './frontends.component.html',
    styleUrls: ['./frontends.component.css', '../../status.component.css'],
    imports: [TranslatePipe]
})
export class FrontendsComponent implements OnInit {
    @Input() frontends?: Frontend[];

    constructor() { }

    ngOnInit(): void {
    }

}
